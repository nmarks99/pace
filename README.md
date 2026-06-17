# pace

`pace` is a single-header, protocol-agnostic client library for EPICS.

It provides a minimal (and opinionated) interface to make your C++ application
EPICS-aware via both Channel Access and pvAccess under a single API. The API
is tailored towards single-threaded applications with a "main loop".

## Requirements

- C++17 compiler
- [EPICS Base](https://epics-controls.org/)
- [PVXS](https://github.com/epics-base/pvxs)

## Example

Bind local variables to PVs and call `sync()` at the start of your main loop
to sync the latest PV values to your local variables:

```cpp
#include "pace.hpp"

int main() {
    pace::Context ctx;

    double temp = 0.0;
    double set = 0.0;
    ctx.connect("IOC:Temperature.VAL").bind(temp);
    ctx.connect("IOC:Setpoint.VAL").bind(set);

    while (true) {
        if (ctx.sync()) {
            printf("Temperature = %.2f\nSetpoint = %.2f", temp, set);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

Synchronous gets and fire-and-forget puts are also supported:

```cpp
std::optional<double> val = ctx.get<double>("IOC:Temperature.VAL");
if (val) {
    printf("Temperature = %.2f\n", *val);
}

ctx.put("IOC:Setpoint.VAL", 350.0);
```
