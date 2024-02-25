import asyncio

class WebSocketEmulator:
    def __init__(self, gateway):
        self.gateway = gateway
        self.on_open = None
        self.on_message = None
        self.is_open = False

    async def connect(self):
        print("WebSocket connection opened.")
        self.is_open = True
        if self.on_open:
            await self.on_open()

    async def send(self, data):
        print(f"Sending data: {data}")
        # Emulate sending data
        await asyncio.sleep(0.5)
        if self.on_message:
            await self.on_message(data)

    async def close(self):
        print("WebSocket connection closed.")
        self.is_open = False

async def main():
    gateway = "ws://example.com/ws"
    rotator_socket = WebSocketEmulator(gateway)

    async def on_open():
        await rotator_socket.send({"message": "Hello, server!"})

    async def on_message(data):
        print(f"Received data: {data}")

    rotator_socket.on_open = on_open
    rotator_socket.on_message = on_message

    await rotator_socket.connect()
    await rotator_socket.send({"command": "start"})
    await rotator_socket.send({"command": "stop"})
    await rotator_socket.close()

asyncio.run(main())
