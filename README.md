<div align="center">
  <a href="https://github.com/gamestdio/colyseus">
    <img src="https://github.com/gamestdio/colyseus/blob/master/media/header.png?raw=true" />
  </a>
  <br>
  <br>
  <a href="https://npmjs.com/package/colyseus">
    <img src="https://img.shields.io/npm/dm/colyseus.svg">
  </a>
  <a href="https://patreon.com/endel" title="Donate to this project using Patreon">
    <img src="https://img.shields.io/badge/patreon-donate-yellow.svg" alt="Patreon donate button" />
  </a>
  <a href="http://discuss.colyseus.io" title="Discuss on Forum">
    <img src="https://img.shields.io/badge/discuss-on%20forum-brightgreen.svg?style=flat&colorB=b400ff" alt="Discussion forum" />
  </a>
  <a href="https://gitter.im/gamestdio/colyseus">
    <img src="https://badges.gitter.im/gamestdio/colyseus.svg">
  </a>
  <h3>
     Multiplayer Game Client for Cocos2d-X. <br /><a href="http://colyseus.io/docs/">View documentation</a>
  <h3>
</div>

## Installation

Download and following [installation instructions](https://github.com/cocos2d/cocos2d-x#download-stable-versions) for [Cocos2d-X](http://www.cocos2d-x.org/download).

## Running the Example

### Running the server locally

Ensure you have [Node v6+](http://nodejs.org/) installed. Then run these
commands in your commandline:

```
cd server
npm install
npm start
```

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

## Usage

### Including `colyseus-cocos2d-x` library.

- Add `Libraries` to your project's `Header Search Paths`
- Add [msgpack-c](https://github.com/msgpack/msgpack-c) to your project's `Header Search Paths`

### Caveats

Mac/iOS: Remove i386 from Valid Architectures.
-- x86_64??

In your scene file, add the following:

```cpp
#include "Colyseus/Client.h";

bool HelloWorldScene::init()
{
    Client::getInstance()->start("ws://localhost:2667", CC_CALLBACK_1(HelloWorldScene::onConnectToServer, this));
}
```

## Contributors

Big thanks to [Hung Hoang](https://github.com/chunho32) for making the initial
implementation of this client.

## License

MIT
