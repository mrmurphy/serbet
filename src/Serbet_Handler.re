open Async;
type complete = Express.complete;

module Status = {
  include Express.Response.StatusCode;
};

type res =
  | BadRequest(string)
  | NotFound(string)
  | Unauthorized(string)
  | OkString(string)
  | OkJson(Js.Json.t)
  | OkBuffer(Node.Buffer.t)
  | StatusString(Status.t, string)
  | StatusJson(Status.t, Js.Json.t)
  | TemporaryRedirect(string)
  | InternalServerError
  | RespondRaw(Express.Response.t => Express.complete)
  | RespondRawAsync(Express.Response.t => promise(Express.complete));

exception HttpException(res);

let abort = res => {
  raise(HttpException(res));
};

type verb =
  | GET
  | POST
  | PUT
  | DELETE;

type guard('a) = Express.Request.t => promise('a);

module ExpressTools = {
  let queryJson = (req: Express.Request.t): Js.Json.t =>
    Obj.magic(Express.Request.query(req));

  [@bs.send] external appSet: (Express.App.t, string, 'a) => unit = "set";

  [@bs.module]
  external middlewareAsComplete:
    (Express.Middleware.t, Express.Request.t, Express.Response.t) =>
    Js.Promise.t(Express.complete) =
    "./middlewareAsComplete.js";
};

let requireHeader: string => guard(string) =
  (headerName, req) => {
    let o = req |> Express.Request.get(headerName);
    switch (o) {
    | Some(h) => async(h)
    | None => abort @@ BadRequest("Missing required header: " ++ headerName)
    };
  };

let requireBody: Decco.decoder('body) => guard('body) =
  (decoder, req) => {
    // We resolve a promise first so that if the decoding
    // fails, it will reject the promise instead of just
    // throwing and requiring a try/catch.
    let%Async _ = async();
    switch (req |> Express.Request.bodyJSON) {
    | Some(rawBodyJson) =>
      switch (decoder(rawBodyJson)) {
      | Error(e) =>
        abort(
          BadRequest(
            "Could not decode expected body: location="
            ++ e.path
            ++ ", message="
            ++ e.message,
          ),
        )
      | Ok(v) => v->async
      }
    | None => abort @@ BadRequest("Body required")
    };
  };

let requireParams: Decco.decoder('params) => guard('params) =
  (decoder, req) => {
    let paramsAsJson: Js.Json.t = Obj.magic(req |> Express.Request.params);
    // We resolve a promise first so that if the decoding
    // fails, it will reject the promise instead of just
    // throwing and requiring a try/catch.
    let%Async _ = async();
    switch (decoder(paramsAsJson)) {
    | Error(e) =>
      abort @@
      BadRequest(
        "Could not decode expected params from the URL path: location="
        ++ e.path
        ++ ", message="
        ++ e.message,
      )
    | Ok(v) => async @@ v
    };
  };

let requireQuery: Decco.decoder('query) => guard('query) =
  (decoder, req) => {
    // We resolve a promise first so that if the decoding
    // fails, it will reject the promise instead of just
    // throwing and requiring a try/catch.
    let%Async _ = async();
    switch (decoder(ExpressTools.queryJson(req))) {
    | Error(e) =>
      abort @@
      BadRequest(
        "Could not decode expected params from query string: location="
        ++ e.path
        ++ ", message="
        ++ e.message,
      )
    | Ok(v) => async @@ v
    };
  };
type handler = Express.Request.t => promise(res);

module type HandlerConfigWithCustomMiddleware = {
  let path: string;
  let verb: verb;
  let middleware: option(array(Express.Middleware.t));
  let handler: handler;
};

module type AppCfg = {let app: Express.App.t;};

module MakeWithCustomMiddleware =
       (AppCfg: AppCfg, Cfg: HandlerConfigWithCustomMiddleware) => {
  let verbFunction =
    switch (Cfg.verb) {
    | GET => Express.App.getWithMany
    | POST => Express.App.postWithMany
    | PUT => Express.App.putWithMany
    | DELETE => Express.App.deleteWithMany
    };

  let resToExpressRes = (res, handlerRes) =>
    switch (handlerRes) {
    | BadRequest(msg) =>
      async @@
      Express.Response.(res |> status(Status.BadRequest) |> sendString(msg))
    | NotFound(msg) =>
      async @@
      Express.Response.(res |> status(Status.NotFound) |> sendString(msg))
    | Unauthorized(msg) =>
      async @@
      Express.Response.(
        res |> status(Status.Unauthorized) |> sendString(msg)
      )
    | OkString(msg) =>
      async @@
      Express.Response.(
        res
        |> status(Status.Ok)
        |> setHeader("content-type", "text/plain; charset=utf-8")
        |> sendString(msg)
      )
    | OkJson(js) =>
      async @@ Express.Response.(res |> status(Status.Ok) |> sendJson(js))
    | OkBuffer(buff) =>
      async @@
      Express.Response.(res |> status(Status.Ok) |> sendBuffer(buff))
    | StatusString(stat, msg) =>
      async @@
      Express.Response.(
        res
        |> status(stat)
        |> setHeader("content-type", "text/plain; charset=utf-8")
        |> sendString(msg)
      )
    | InternalServerError =>
      async @@
      Express.Response.(
        res |> sendStatus(Express.Response.StatusCode.InternalServerError)
      )
    | StatusJson(stat, js) =>
      async @@ Express.Response.(res |> status(stat) |> sendJson(js))
    | TemporaryRedirect(location) =>
      async @@
      Express.Response.(
        res
        |> setHeader("Location", location)
        |> sendStatus(StatusCode.TemporaryRedirect)
      )
    | RespondRaw(fn) => async @@ fn(res)
    | RespondRawAsync(fn) => fn(res)
    };

  let wrappedHandler = (_next, req, res) => {
    let handleOCamlError =
      [@bs.open]
      (
        fun
        | HttpException(handlerResponse) => handlerResponse
      );
    let handleError = error => {
      switch (handleOCamlError(error)) {
      | Some(res) => res->async
      | None =>
        switch (Obj.magic(error)##stack) {
        | Some(stack) => Js.log2("Unhandled internal server error", stack)
        | None => Js.log2("Unhandled internal server error", error)
        };
        InternalServerError->async;
      };
    };

    switch (Cfg.handler(req)) {
    | exception err =>
      let%Async r = handleError(err);
      resToExpressRes(res, r);
    | p =>
      let%Async r = p->catch(handleError);
      resToExpressRes(res, r);
    };
  };
  let expressHandler = Express.PromiseMiddleware.from(wrappedHandler);

  AppCfg.app->verbFunction(
    ~path=Cfg.path,
    Belt.Array.concat(
      Cfg.middleware->Belt.Option.getWithDefault([||]),
      [|expressHandler|],
    ),
  );
};

module type HandlerConfig = {
  let path: string;
  let verb: verb;
  let handler: handler;
};

[@bs.module]
external jsonParsingMiddleware: Express.Middleware.t =
  "./json-parsing-middleware.js";

module Make = (AppCfg: AppCfg, Cfg: HandlerConfig) => {
  include MakeWithCustomMiddleware(
            AppCfg,
            {
              let path = Cfg.path;
              let verb = Cfg.verb;
              let handler = Cfg.handler;
              let middleware =
                Some([|
                  // By default we parse JSON bodies
                  jsonParsingMiddleware,
                |]);
            },
          );
};

module type HandlerConfigWithBody = {
  let path: string;
  let verb: verb;

  type body;
  let body_decode: Decco.decoder(body);

  type response;
  let response_encode: Decco.encoder(response);

  let handler: (body, Express.Request.t) => promise(response);
};

module MakeJson = (AppCfg: AppCfg, Cfg: HandlerConfigWithBody) => {
  include Make(
            AppCfg,
            {
              let path = Cfg.path;
              let verb = Cfg.verb;
              let handler = req => {
                let%Async body = requireBody(Cfg.body_decode, req);
                let%Async response = Cfg.handler(body, req);
                async(OkJson(Cfg.response_encode(response)));
              };
            },
          );
};