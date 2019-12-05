// Node.js by default doesn't have useful stack traces when it comes to async code. But asnc stack traces are essential for debugging when writing a real server. This library uses Bluebird to replace the global promise implementation which gives async stack traces by default in development.
// Copy this file into your project (it's not meant to be runnable from within this project), run the compiled JS artifact, and then open localhost:3002 in your browser, you'll see a detailed stack trace that leads you back through the actual execution path of the code, thanks to Bluebird's async stack traces.

open Async;

type user = {
  id: int,
  email: string,
};

let saveUserInDatabase = (_user: user): promise(unit) => {
  Js.Exn.raiseError("This is a simulated error from the database driver");
};

let processUser = user => {
  async @@ {...user, id: user.id + 2};
};

let newUser = () => {
  async @@ {id: 1, email: ""};
};

open Handler;
module Endpoint =
  Handler.Make({
    let path = "/";
    let verb = GET;
    let handler = _req => {
      let%Async user = newUser();
      let%Async user2 = processUser(user);
      let%Async _ = saveUserInDatabase(user2);
      async @@ OkString("Done");
    };
  });

let app = Express.App.make();
let router = Express.Router.make();
router->Endpoint.use;

app->Express.App.useRouter(router);
app->Express.App.listen(~port=3002, ());
