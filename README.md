# 转发服务器文档

## 基本接口
```
bool Start(char*ip = 0, int port = 12345);
virtual bool validate(uint32_t uid1, uint32_t sessionId, uint32_t uid2, std::string *url);
void Close();;
```
##安全验证机制
默认采用http get请求回调，可通过基层CRelayServer并override虚函数使用自己的验证方法

登陆转发服务器时：
    请求为url?uid=xxx&sessionId=xxx

查询是否有转发权限时：
    请求为url?uid1=xxx&sessionId=xxx&&uid2=xxx

##配置文件格式

