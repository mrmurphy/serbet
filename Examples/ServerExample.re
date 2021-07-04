open Async;
open Serbet.Endpoint;

let hello =
  Serbet.endpoint({
    verb: GET,
    path: "/",
    handler: _req => {
      // In case you've never seen it before, @@ evaluates the result of its right-hand side, and passes the result as the only argument to whatever function is on the left-hand side.
      async @@ OkString("Hello, world");
    },
  });

module HelloJson = {
  // We need to specify request and response body types and decoders for the handler function! Decco generates the codecs for us automatically.
  [@decco.decode]
  type body_in = {name: string};

  [@decco.encode]
  type body_out = {greeting: string};

  let endpoint =
    Serbet.jsonEndpoint({
      verb: POST,
      path: "/json",
      body_in_decode,
      body_out_encode,

      // Now the parsed body gets passed in as the first argument to the handler, and we can return the unencoded response body from the function, and the framework takes care of the rest.
      handler: (body, _req) => {
        async @@ {greeting: "Hello " ++ body.name};
      },
    });
};

module HelloQuery = {
  [@decco.decode]
  type query = {name: string};

  let endpoint =
    Serbet.endpoint({
      verb: GET,
      path: "/hello/query",

      handler: req => {
        let%Async query = req.requireQuery(query_decode);
        OkString("Hello there, " ++ query.name)->async;
      },
    });
};

module AuthorizedHello = {
  [@decco.decode]
  type query = {name: string};

  type user = {name: string};

  // All we need to do for authentication and authorization is make a function
  // that takes the request, and either returns a promise with the user, or aborts
  // with some error message to the caller.
  let authorize = req => {
    let%Async authHeader = req.requireHeader("authorization");
    switch (authHeader) {
    | "Bearer: frodobaggins" => {name: "frodo"}->async
    | _ => abort @@ Unauthorized("Only the ring bearer is allowed here")
    };
  };

  let endpoint =
    Serbet.endpoint({
      verb: GET,
      path: "/hello",

      handler: req => {
        let%Async user = authorize(req);
        OkString("Hello there, " ++ user.name)->async;
      },
    });
};

// This makes an application, registers the endpoints, and starts the app.
// If no port is specified, the PORT env var is used, or if that's not specified, some other default.
let app =
  Serbet.application(
    ~port=3110,
    [hello, HelloJson.endpoint, HelloQuery.endpoint],
  );