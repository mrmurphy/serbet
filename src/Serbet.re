module App = (()) => {
  let app = Express.App.make();
  let router = Express.Router.make();
  app->Express.App.useRouter(router);

  include Serbet_Handler;

  module Handle =
    Serbet_Handler.Make({
      let app = app;
    });

  module HandleJson =
    Serbet_Handler.MakeJson({
      let app = app;
    });

  let start = (~port=?, ()) => {
    app->Express.App.listen(
      ~port=port->Belt.Option.getWithDefault(3000),
      ~onListen=_ => {Js.log2("Server listening on port", port)},
      (),
    );
  };
};