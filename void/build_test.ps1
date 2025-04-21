clang switch.cpp switch.s -o switch.exe -Xlinker /LARGEADDRESSAWARE:NO -Xlinker /INCREMENTAL:NO
clang switch_test.cpp switch.s -o switch_test.exe -Xlinker /LARGEADDRESSAWARE:NO
