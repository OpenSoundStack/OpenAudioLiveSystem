# Open Audio Live System
## Introduction
Open Audio Live System is a highly customisable live audio environment meant to offer an open alternative for professional
live audio context with real-world usage in mind.

This system is divided in four major parts :
 - Hardware : IO Boards, DSP Boards, stagebox enclosures, Control surface...
 - Control UI
 - DSP implementation targeted for high performance ARM embedded systems
 - L2 Network protocol that allows audio transport, node control, node discovery and PTP clock sharing across an Ethernet
 switched network

This repo contains the major "software defined" components of the console that rely on [OpenDSP](https://github.com/OpenSoundStack/OpenDSP)
and [OpenAudioNetwork](https://github.com/OpenSoundStack/OpenAudioNetwork/). These components are the console UI,
the console DSP and the plugin loader.

## Repo organisation

```
.
├── OpenAudioNetwork      # L2 Networking module
├── OpenDSP               # DSP base library
├── coreui                # Console UI
│   ├── core              # Wrappers for OALS objects around Qt framework
│   ├── pipes             # Baked-in pipes UI implementation
│   ├── surface_config    # Console settings
│   │   ├── pipes.json    # Processing pipes templates config
│   │   └── surface.json  # Contains the surface networking and plugin configuration
│   └── ui                # Generic UI code handling pages, basic menus, ...
├── debugger              # Small debugging app for networking
├── engine                # DSP engine
│   └── piping            # Baked in pipes DSP implementation
├── io_sim                # (Deprecated) Small user-space virtual sound card
└── plugins               # Plugin loader and built-in plugins folder
    ├── builtin           # Builtin plugins code
    └── loader            # Plugin loader and interface library
```

## Compiling
This console system software tries to minimise the count of external dependencies. The only needed dependency is the Qt 6
framework. Otherwise, it compiles like any standard cmake project.

### Cloning
```shell
$> git clone git@github.com:OpenSoundStack/OpenAudioLiveSystem.git --recurse-submodules
```

### Compiling
```shell
$> mkdir ./build && cd ./build
$> cmake <path_to_cloned_repo>
$> cmake --build . --target all
```

## Running the console software
As this project is made for specific hardware, it can be a bit tricky to get it running on your own machine. There are
two ways to do that.

There are no kernel drivers developed yet for the networking part, because of that the engine and the UI must run as root
because the network protocol is a L2 protocol relying on raw packet sockets.

> [!TIP]
> If the setup is for development purposes, it is highly recommended to set up remote debug session with the console binaries
> not running on the host machine.

> [!WARNING]
> Don't forget to do a cmake install before running to ensure the plugins are properly compiled and installed. 
> You'll encounter errors if there are mismatch between plugin lists on the DSP and the UI. 

> [!CAUTION]
> The network protocol is NOT routable. No IP tunnelling is provided at the moment. The network the console is working on
> **must** be switched.

There are no support (yet) for IO emulation. However `io_sim` can be used as a template to load and send in loop STEMs on
the console network.

### The virtual way
That way is mainly for those who just wants a debugging environment. The DSP must run in a virtual machine running the
Linux flavour of your choice, ideally using an RT patched kernel. The virtual machine must be connected on a virtual network
with the host. The host will run the UI.

If everything went OK, both apps should see each-others on the network. It will be notified in the logs.

### The real-hardware way
If you happen to have two physical machines running Linux, a network switch and optionally IO Boards — they can always be
simulated in software —  you can run the console in the way it was meant to. 

Run the DSP engine on one machine, the UI on the other and connect them on the same **switched** network.