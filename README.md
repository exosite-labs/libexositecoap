**NOTICE: This library is not finished, while I have been running it very
successfully on an embedded device for the three last months without any issue,
there are no tests in place and the API is not stabilized. In fact we are
considering rewriting the whole interface. If you are interested in using this
library, or if you have strong opinions on what you'd like to see in it, please
get in contact with us and we can talk about roadmap.**

# libexositecoap

This is the C library for interacting with the Exosite One Platform. It aims to
be fast, efficient, and flexible. Platform operations are queued up and then
performed with non-blocking calls so that your application can continue to do
local processing while waiting for the network transactions to complete.

## Status

This is a very early release, there will be bugs. If you find one, please open
an issue on https://github.com/exosite-labs/libexositecoap/issues. Pull requests
are also highly encouraged. At some point in the future you will be required to
show a test case that fails before your patch and succeeds after it, but there
aren't any tests yet, so just describe the problem.

Because this is an alpha release, please don't use it in production.
Backwards-incompatible changes will very likely have to be made when bugs are
found, but you are highly encouraged to use it for testing. The examples do
work and the changes to the interface won't be too substantial.

Only a limited set of the final features have been implemented. Currently, reads
writes, and subscribes work. You can not yet activate or interact with content.

The inline documentation is incomplete and is probably wrong in many places,
don't pay too much attention to it. You should be able to figure out most of the
usage from the examples and the function prototypes.

## About

This library uses the concept of "operations" (ops) for all interactions with
the OneP. One operation is a single read, write, or subscription of a single
dataport. There are two classes of ops, single shot and sustained. Read and
write are single shot operations. That means that you queue the operation then
process until it either succeeds or fails. Subscribe is a sustained operation,
that means that you queue it once then the library will do everything in it's
power to keep it active.

## Usage

This section won't explain everything, at least not until we decide that the API
is stable, see the examples folder for real uses.

### Most Common Use

You have to hold the memory for the ops, it's up to you if you want to use
malloc, create an array of `exo_op`s then call `exo_op_init()` on that array to
initialize it. Next add any sustained operations that you want. Then add any
one-shot operations you want to call immediately. Once you have all the
operations you want queued up, call `exo_operate()` until it returns `EXO_IDLE`,
this means that it has nothing more to do and all of the queued single-shot
operations have succeeded or failed and that all of the queued sustained
sustained operations are in a waiting state and are not expecting any immediate
responses. This is the time to do any operations that will take more than a
couple hundred milliseconds.

### When Not Using Provisioning with Examples

If you're planning on testing the included example
(`make posixsubscribe && ./posixsubscribe`) and you're not using the
provisioning system you'll need to paste your CIK into a file named `cik` to
fool the library into thinking that it already activated itself.
