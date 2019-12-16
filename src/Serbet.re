module Express = Express;

module Endpoint = Serbet_Endpoint;
type endpoint = Endpoint.endpoint;

let endpoint = Endpoint.endpoint;
let jsonEndpoint = Endpoint.jsonEndpoint;

type application = {
  expressApp: Express.App.t,
  router: Express.Router.t,
};

let application = (~port=?, endpoints: list(endpoint)) => {
  let app = Express.App.make();
  let router = Express.Router.make();
  app->Express.App.useRouter(router);

  endpoints->Belt.List.forEach(ep => {ep.useOnRouter(router)});

  let defaultPort =
    Node.Process.process##env
    ->Js.Dict.get("PORT")
    ->Belt.Option.map(a => Js.Float.fromString(a)->int_of_float)
    ->Belt.Option.getWithDefault(3000);

  let effectivePort = port->Belt.Option.getWithDefault(defaultPort);

  app->Express.App.listen(
    ~port=effectivePort,
    ~onListen=_ => {Js.log2("Server listening on port", effectivePort)},
    (),
  )
  |> ignore;

  {expressApp: app, router};
};