open Async;

module App =
  Server.Make({});

module Hello =
  App.Handle({
    open App;
    let verb = GET;
    let path = "/";
    let handler = _req => {
      async(OkString("Hello, world"));
    };
  });

App.start(~port=3110, ());