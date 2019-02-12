<div align="center">
  <a href="https://github.com/colyseus/colyseus">
    <img src="https://github.com/colyseus/colyseus/blob/master/media/header.png?raw=true" />
  </a>
  <br>
  <br>
  <a href="https://npmjs.com/package/colyseus">
    <img src="https://img.shields.io/npm/dm/colyseus.svg?style=for-the-badge&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAQAAAC1+jfqAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAAmJLR0QAAKqNIzIAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAAHdElNRQfjAgETESWYxR33AAAAtElEQVQoz4WQMQrCQBRE38Z0QoTcwF4Qg1h4BO0sxGOk80iCtViksrIQRRBTewWxMI1mbELYjYu+4rPMDPtn12ChMT3gavb4US5Jym0tcBIta3oDHv4Gwmr7nC4QAxBrCdzM2q6XqUnm9m9r59h7Rc0n2pFv24k4ttGMUXW+sGELTJjSr7QDKuqLS6UKFChVWWuFkZw9Z2AAvAirKT+JTlppIRnd6XgaP4goefI2Shj++OnjB3tBmHYK8z9zAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDE5LTAyLTAxVDE4OjE3OjM3KzAxOjAwGQQixQAAACV0RVh0ZGF0ZTptb2RpZnkAMjAxOS0wMi0wMVQxODoxNzozNyswMTowMGhZmnkAAAAZdEVYdFNvZnR3YXJlAHd3dy5pbmtzY2FwZS5vcmeb7jwaAAAAAElFTkSuQmCC">
  </a>
  <a href="https://patreon.com/endel" title="Donate to this project using Patreon">
    <img src="https://img.shields.io/badge/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.herokuapp.com%2Fendel&style=for-the-badge" alt="Patreon donate button"/>
  </a>
  <a href="https://discuss.colyseus.io" title="Discuss on Forum">
    <img src="https://img.shields.io/badge/discuss-on%20forum-brightgreen.svg?style=for-the-badge&colorB=0069b8&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAQAAAC1+jfqAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAAmJLR0QAAKqNIzIAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAAHdElNRQfjAgETDROxCNUzAAABB0lEQVQoz4WRvyvEARjGP193CnWRH+dHQmGwKZtFGcSmxHAL400GN95ktIpV2dzlLzDJgsGgGNRdDAzoQueS/PgY3HXHyT3T+/Y87/s89UANBKXBdoZo5J6L4K1K5ZxHfnjnlQUf3bKvkgy57a0r9hS3cXfMO1kWJMza++tj3Ac7/LY343x1NA9cNmYMwnSS/SP8JVFuSJmr44iFqvtmpjhmhBCrOOazCesq6H4P3bPBjFoIBydOk2bUA17I080Es+wSZ51B4DIA2zgjSpYcEe44Js01G0XjRcCU+y4ZMrDeLmfc9EnVd5M/o0VMeu6nJZxWJivLmhyw1WHTvrr2b4+2OFqra+ALwouTMDcqmjMAAAAldEVYdGRhdGU6Y3JlYXRlADIwMTktMDItMDFUMTg6MTM6MTkrMDE6MDAC9f6fAAAAJXRFWHRkYXRlOm1vZGlmeQAyMDE5LTAyLTAxVDE4OjEzOjE5KzAxOjAwc6hGIwAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAAASUVORK5CYII=" alt="Discussion forum" />
  </a>
  <a href="https://discord.gg/RY8rRS7">
    <img src="https://img.shields.io/discord/525739117951320081.svg?style=for-the-badge&colorB=7581dc&logo=discord&logoColor=white">
  </a>
  <h3>
     Multiplayer Game Client for <a href="https://github.com/cocos2d/cocos2d-x">Cocos2d-x</a>. <br /><a href="http://colyseus.io/docs/">View documentation</a>
  </h3>
</div>

- [Usage](#installation)
- [Installation](#installation)
- [Example](#example)

## Usage

Add the `Source/` directory to your project's `Header Search Paths`.

Below you can see how to use the `Client`, matchmake into a `Room`, and send and
receive messages from the connected room.

```cpp
#include "Colyseus/Client.h";

Client* client;
Room* room;

bool HelloWorld::init()
{
    client = new Client("ws://localhost:2667");
    client->onOpen = CC_CALLBACK_0(HelloWorld::onConnectToServer, this);
    client->connect();
}

void HelloWorld::onConnectToServer()
{
    log("Colyseus: CONNECTED TO SERVER!");
    room->onMessage = CC_CALLBACK_2(HelloWorld::onRoomMessage, this);
    room->onStateChange = CC_CALLBACK_1(HelloWorld::onRoomStateChange, this);

    room->listen("players/:id", [this](std::map<std::string, std::string> path, PatchObject patch) -> void {
        std::cout << "CALLBACK FOR 'players/:id' >>" << std::endl;
        std::cout << "OPERATION: " << patch.op << std::endl;
        std::cout << "PLAYER ID:" << path.at(":id") << std::endl;
        std::cout << "VALUE: " << patch.value << std::endl;
    });
}

void HelloWorld::onRoomMessage(Room* sender, msgpack::object message)
{
    std::cout << "!! HelloWorld::onRoomMessage !!" << std::endl;
    std::cout << message << std::endl;
}

void HelloWorld::onRoomStateChange(Room* sender)
{
    std::cout << "!! HelloWorld::onRoomStateChange !!" << std::endl;
    std::cout << sender->state->get() << std::endl;
}
```


## Installation

Download and following [installation instructions](https://github.com/cocos2d/cocos2d-x#download-stable-versions) for [Cocos2d-X](http://www.cocos2d-x.org/download).

## Example

The example is using [colyseus-examples](https://github.com/colyseus/colyseus-examples) as a server (`02-state-handler.ts` example). Please follow installation instructions from [colyseus-examples](https://github.com/colyseus/colyseus-examples)'s README.

### Running the client

From the `Example` directory, run the `cocos run -p {platform-id}` command,
e.g.:

```
# running on windows
cocos run -p win32
```

```
# running on mac
cocos run -p mac
```

## Contributors

Big thanks to [Hung Hoang](https://github.com/chunho32) for making the initial
implementation of this client.

## License

MIT
