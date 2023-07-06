# ComputerNetwork-Labs
## LAB1 TCP单进程服务器与客户端简单回声
## LAB2 TCP多进程服务器与客户端简单回声
### 易错点
- 注意SCOKET系统调用的顺序
- 计算整个PDU的长度时，不能使用strlen()，因为报文头部的VCD或者CID可能包含'\0'，导致strlen()计算出的长度不正确