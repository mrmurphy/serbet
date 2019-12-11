open Async;

module App =
  Serbet.App({});

open App;

// This makes and registers an endpoint, all in one go.
module Hello =
  App.Handle({
    let verb = GET;
    let path = "/";
    let handler = _req => {
      // In case you've never seen it before, @@ evaluates the result of its right-hand side, and passes the result as the only argument to whatever function is on the left-hand side.
      async @@ OkString("Hello, world");
    };
  });

// This is a convenience module which automatically parses and json body and encodes a json response.
module HelloJson =
  App.HandleJson({
    let verb = POST;
    let path = "/json";

    // The HandleJson module config requires us to specify a body type (with its decoder, which is provided by the @decco decorator here.) and a response type.
    [@decco.decode]
    type body = {name: string};

    [@decco.encode]
    type response = {greeting: string};

    // Now the parsed body gets passed in as the first argument to the handler.
    let handler = (body, _req) => {
      async @@ {greeting: "Hello " ++ body.name};
    };
  });

module HelloQuery =
  App.Handle({
    let verb = GET;
    let path = "/hello/query";

    [@decco.decode]
    type query = {name: string};

    let handler = req => {
      let%Async query = requireQuery(query_decode, req);

      OkString("Hello there, " ++ query.name)->async;
    };
  });

App.start(~port=3110, ());