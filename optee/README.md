
# Trust.io OP-TEE changes

This folder contains the modifications we made to the op-tee sources and general technique on how trust.io is implemented on [OP-TEE](https://optee.readthedocs.io/en/latest/).

We have tested our changes on a Hikey Board.

The data flow of our implementation is as shown below:

![OPTEETrustIO](https://github.com/ucsb-seclab/trust.io/raw/master/optee/pics/OPTEESetup.png)

The numbers on the arrows show the order in which the messages will be exchanged between components. There are two numbers separated by `/` where the first number is for the forward direction, and the second is for the reverse.

We have changed the following components of OP-TEE debian:

### Debian linux
* Base commit hash: `04ec80a78dbc970cf921abc02910d2148cec6dbb`
* The changed files are in the folder: `linux`
* The unified diff is in the file: `diffs/linux.diff`

### OP-TEE Trusted OS
* Base commit hash: `94ee4938f7f544d07d11721a355f445c020663ca`
* The changed files are in the folder: `optee_os`
* * The unified diff is in the file: `diffs/optee_os.diff`

### Op-tee client for TEE Supplicant
* Base commit hash: `09b69afa5e9e74aac39e383d74f14b4d61c90476`
* The changed files are in the folder: `optee_client`
* The unified diff is in the file: `diffs/optee-client.diff`

## Building and running
__Note: The IP addresses and port numbers in `gpio_dev` server, `gpio_client`, `tee_supplicant`, and `gpio_callback` are hardcoded. Make sure that you change them to the corresponding external component.
For example: The IP address and port number in `tee_supplicant` should be that of `gpio_callback` server.__

Apply the required modifications to the various components of OP-TEE using the above information and follow instructions from the original op-tee repository to build.

### Running

__Note: make sure that `gpio_client`, `gpio_callback` and the board can communicate with each other through IP.__

* Copy the `gpio_dev`  sources (under `gpio/board` folder) into the Hikey board and run the corresponding `gpio_dev` server on the hikey board.
* Run `gpio_callback` server on an IP enabled machine.
* Run the required `gpio_client`, you should see LED light blinking and callback server serving requests.
