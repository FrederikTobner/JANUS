# include — Global Headers

Global, cross-cutting header files used throughout JANUS.
The headers are split into two parts.
First of all there is the [User API](./uapi/README.md), which defines types that can be used by programms running in user space to interact with the kernel.
This user api is in turn used in the [global JANUS kernel include layer](./janus/README.md) to define types that are used in the kernel internally, its own compiler attributes, version etc.
