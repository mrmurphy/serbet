## Serbet (like sorbet, but a server. Get it? 🍨)

A simple way to set up an Express.js server on top of Node.js using ReasonML.

Documentation planned! For now, you can read the source code or take a look at this:

```reason
open Async;

module App =
  Serbet.App({});

// This makes and registers an endpoint, all in one go.
module Hello =
  App.Handle({
    open App;
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
    open App;
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

App.start(~port=3110, ());
```

That's a real quick-and-dirty example of getting a server going.

Here are some quick tips:

- `decco` and `bs-let` are peer dependencies. Don't forget to install them or things won't work!
- There are helper functions for parsing path elements, queries, and bodies, like this:

```reason
module Hello =
  App.Handle({
    open App;

    [@decco]
    type path = {
      userId: string
    }

    let verb = GET;
    let path = "/:userId";
    let handler = req => {
      let%Async path = requirePath(path_decode, req);
      async(OkString("Hello, world"));
    };
  });
```

- If you want to add a guard (like authorization), write a function that takes a request, and returns a promise of a user, and put it at the top of the function, just like we did with `path` above.
- You can abort the request at any time with the `abort` function, which takes a `response` type. (This uses JS promise exceptions)

Those are the very sparse docs for now. It looks like a side project, but we've been using the these patterns in production already and are quite happy with the results so far.
