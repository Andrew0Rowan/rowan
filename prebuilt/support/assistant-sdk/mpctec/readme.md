readme
======

## 目录说明

├── algo-data-server				// 这个是算法进程，后台启动即可
├── demo
│   ├── algo-data-client.h
│   ├── algo-data-client-test
│   ├── algo-data-client-test.cpp	//这个是测试demo，包含sensory唤醒和保存音频
│   ├── dictionary.h
│   ├── iniparser.h
│   ├── Makefile
│   ├── output_lock_angle.cpp
│   ├── output_lock_angle.h
│   ├── queue.h
│   ├── res.ini						//配置文件，为了测试alexa效果，注意代码里面指定的目录
│   ├── sensory						//sensory相关文件
│   │   ├── libthf.a
│   │   ├── model
│   │   │   ├── thfft_alexa_a_enus_v3_1mb_am.raw
│   │   │   └── thfft_alexa_a_enus_v3_1mb_search_9.raw
│   │   ├── sensorytypes.h
│   │   └── trulyhandsfree.h
│   └── wav_helper.h
├── lib								//需要引用的库文件
├── mnpc_v2.dat						//这个文件放在/data/mnano目录下
└── readme.md

> note: 如果不需要保存音频，在Makefile中可以去掉`-lbase_lib`的引用, base_lib仅用于测试保存音频用

> note: 在`algo-data-client-test.cpp`中的宏`READ_ALEXA_CONF`只用于测试，确定配置文件后，可以删除该宏和相关的代码，
>       同时删除dictionary.h和iniparser.h头文件，以及libiniparser.so库文件


