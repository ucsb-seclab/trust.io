# Setup a Development Environment

1. Install Vivado

2. Install the required drivers
```bash
$ ./tools/install_drivers.sh
```
3. Launch Vivado and open hardware project (File|Open Project)
 - Select the .xpr file in hardware/zc702_3
4. Export hardware (File|Export Hardware) to *./software/*

5. Open the SDK (File|Launch SDK)
 - Exported location: *./software/*
 - Workspace: *./software/*
6. Import existing projects (File|Open projects from File System...)
 - Import source: *./software/*

## Useful debugging and testing parameters

Throughout the code I've place numerous #ifdefs to make our debugging life easier.
These include:
 * TRUST_IO - Trust.IO enabled when this flag is set
 * DEBUG - Will print useful debug information
 * TIMING - Will print timing information so that can measure our impact

To enable/disable any of these from the SDK:
 1. Right click on the the project name and select *C/C++ Build Settings*.
 2. Under ARM v7 gcc compiler, select Symbols
 3. In *Defined symboles (-D)* add or remove the desired definitions


# Useful Links

https://community.arm.com/tools/f/discussions/453/running-trustzone

http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0333h/Chdfjdgi.html

https://genode.org/documentation/articles/trustzone
