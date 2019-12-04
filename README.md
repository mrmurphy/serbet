## Serbet (like sorbet, but a server. Get it? 🍨)

A simple way to set up an Express.js server on top of Node.js using ReasonML.

Documentation planned! For now, you can read the source code or take a look at this:

```reasonml
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
```

That's a real quick-and-dirty example of getting a server going.

Here are some quick tips:

- `decco` and `bs-let` are peer dependencies. Don't forget to install them or things won't work!
- There are helper functions for parsing path elements, queries, and bodies, like this:

```reasonml
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
- If you have a greenfield server and want to just use Json everywhere, you can make your life a little easier with `HandleJson`:

```reasonml
module Hello =
  App.Handle({
    open App;

    [@decco.decoder]
    type body = {
      userId: string
    }

    [@decco.encoder]
    type response = {
      firstName: string
    };

    let verb = POST;
    let path = "/";
    let handler = (body, _req) => {
      async({firstName: "bob"});
    };
  });
```

Those are the very sparse docs for now. It looks like a side project, but we've been using the these patterns in production already and are quite happy with the results so far.
