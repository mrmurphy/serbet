![Logo Artwork](https://user-images.githubusercontent.com/1227109/98459287-9a470780-213d-11eb-8cb3-20ba97c5cac9.png)

## Serbet (like sorbet, but a server. Get it?)

A simple way to set up an Express.js server on top of Node.js using ReasonML.

Documentation planned! For now, you can read the source code in ServerExample.re


Here are some quick tips:

- `decco` and `bs-let` are peer dependencies. Don't forget to install them or things won't work!
- There are helper functions for parsing path elements, queries, and bodies, like this:
- If you want to add a guard (like authorization), write a function that takes a request, and returns a promise of a user, and put it at the top of the function, just like we did with `path` above.
- You can abort the request at any time with the `abort` function, which takes a `response` type. (This uses JS promise exceptions)

Those are the very sparse docs for now. It looks like a side project, but we've been using the these patterns in production already and are quite happy with the results.

Thanks so much to [Laura Randle](https://www.laurajrandle.com) for the artwork.
