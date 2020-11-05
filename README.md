# east

A rpc tool for Linux machine which can be used to monitor, update, communication, and so on.

```
east tool ~('_')=====b
```

### Dependency

```shell
sudo apt-get update
sudo apt-get install libev-dev 
sudo apt-get install linux-tools-common linux-tools-generic linux-tools-`uname -r`
sudo apt-get install libncurses5-dev
```

### Run

After launch `east`, it will be connected via JsonRpc. You can use the linux command `nc` to testing quickly.

For example, send the rpc method "sayHello".

```shell
echo "{\"method\":\"sayHello\"}" | nc 127.0.0.1 6574
```

You will get the json data like this.

```json
{
	"result":	"Hello!"
}
```


### Reference

- [jsonrpc-c](https://github.com/hmng/jsonrpc-c)
- [lepd](https://github.com/linuxep/lepd)

