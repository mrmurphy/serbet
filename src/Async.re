// This file is designed to be opened for entire modules.

// Using Bluebird for the global promise implementation allows actually useful
// stack traces to be generated for debugging runtime issues.
%bs.raw
{|global.Promise = require('bluebird')|};

let let_ = (p, cb) => Js.Promise.then_(cb, p);

let async = a => Js.Promise.resolve(a);

type promise('a) = Js.Promise.t('a);

let catch = (p, cb) => Js.Promise.catch(cb, p);