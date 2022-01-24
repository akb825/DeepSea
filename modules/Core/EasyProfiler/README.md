# EasyProfiler

This module integrates the DeepSea profiling interface with [easy_profiler](https://github.com/yse/easy_profiler). The library is included as a submodule to compile in support, currently using [version 2.0.1](https://github.com/yse/easy_profiler/releases/tag/v2.0.1). This will automatically include the code required to instrument the code, but the desktop application to view the results must either be downloaded from the linked release or built separately.

Most applications that want to enable profiling will want to call these two functions at the start:

```
dsEasyProfiler_start(false);
dsEasyProfiler_startListening(DS_DEFAULT_EASY_PROFILER_PORT);
```

This will start profiling without enabling full capture and allow connecting through the GUI tool at the default port. If you want to get full profiling information from the very start of the application you can call `dsEasyProfiler_start(true)` install, but note that this will build up memory until the profiling information can be dumped.
