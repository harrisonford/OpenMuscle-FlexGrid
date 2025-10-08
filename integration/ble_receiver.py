#!/usr/bin/env python3
"""
OpenMuscle BLE Receiver and Action Controller

This script connects to the OpenMuscle wristband via Bluetooth LE,
receives gesture data, and triggers actions via various APIs and services.

Requirements:
    pip install bleak asyncio aiohttp openai anthropic

Usage:
    python ble_receiver.py --device "OpenMuscle-FlexGrid"

Author: OpenMuscle Community
License: MIT
"""

import asyncio
import json
import sys
import argparse
from datetime import datetime
from bleak import BleakClient, BleakScanner
import aiohttp

# BLE UUIDs (must match firmware)
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
GESTURE_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
COMMAND_CHAR_UUID = "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"

# Configuration
CONFIG = {
    "openai_api_key": "your-openai-api-key-here",
    "anthropic_api_key": "your-anthropic-api-key-here",
    "philips_hue_bridge": "192.168.1.100",
    "philips_hue_token": "your-hue-token-here",
    "uber_api_key": "your-uber-api-key-here",
    "home_assistant_url": "http://homeassistant.local:8123",
    "home_assistant_token": "your-ha-token-here",
}


class GestureActionController:
    """
    Maps gestures to actions and executes them
    """
    
    def __init__(self, config):
        self.config = config
        self.last_gesture = None
        self.last_gesture_time = None
        self.gesture_cooldown = 1.0  # seconds
        
    async def handle_gesture(self, gesture_data):
        """
        Main gesture handler - routes to appropriate action
        """
        gesture = gesture_data.get("gesture")
        confidence = gesture_data.get("confidence", 0)
        timestamp = gesture_data.get("timestamp", 0)
        imu = gesture_data.get("imu", {})
        
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Gesture: {gesture} ({confidence*100:.1f}%)")
        
        # Debounce - avoid triggering same gesture multiple times
        if gesture == self.last_gesture:
            if self.last_gesture_time and (datetime.now().timestamp() - self.last_gesture_time < self.gesture_cooldown):
                return
        
        self.last_gesture = gesture
        self.last_gesture_time = datetime.now().timestamp()
        
        # Route gesture to action
        try:
            if gesture == "point_up":
                await self.control_light("ceiling_light", "on", imu)
            elif gesture == "point_down":
                await self.control_light("ceiling_light", "off", imu)
            elif gesture == "point_left":
                await self.control_device_in_direction("left", imu)
            elif gesture == "point_right":
                await self.control_device_in_direction("right", imu)
            elif gesture == "fist_close":
                await self.call_uber()
            elif gesture == "swipe_left":
                await self.media_control("previous")
            elif gesture == "swipe_right":
                await self.media_control("next")
            elif gesture == "twist_cw":
                await self.unlock_door()
            elif gesture == "pinch":
                await self.unlock_phone()
            else:
                print(f"  → No action mapped for '{gesture}'")
                
        except Exception as e:
            print(f"  ✗ Error executing action: {e}")
    
    # ===== ACTION IMPLEMENTATIONS =====
    
    async def control_light(self, light_id, action, imu):
        """
        Control smart light via Philips Hue or Home Assistant
        """
        print(f"  → Controlling light: {light_id} -> {action}")
        
        # Option 1: Philips Hue
        hue_bridge = self.config.get("philips_hue_bridge")
        hue_token = self.config.get("philips_hue_token")
        
        if hue_bridge and hue_token:
            url = f"http://{hue_bridge}/api/{hue_token}/lights/1/state"
            payload = {"on": action == "on"}
            
            async with aiohttp.ClientSession() as session:
                async with session.put(url, json=payload) as response:
                    if response.status == 200:
                        print(f"  ✓ Light turned {action}")
                    else:
                        print(f"  ✗ Failed to control light: {response.status}")
        
        # Option 2: Home Assistant
        elif self.config.get("home_assistant_url"):
            await self.call_home_assistant_service(
                "light.turn_on" if action == "on" else "light.turn_off",
                {"entity_id": f"light.{light_id}"}
            )
        
        else:
            print("  ⚠ No smart light system configured (simulation)")
    
    async def control_device_in_direction(self, direction, imu):
        """
        Control device in pointed direction using IMU orientation
        """
        pitch = imu.get("pitch", 0)
        roll = imu.get("roll", 0)
        
        print(f"  → Controlling device in direction: {direction} (pitch={pitch:.1f}°, roll={roll:.1f}°)")
        
        # Determine which device is in that direction
        # This would require spatial mapping in a real implementation
        device_map = {
            "left": "lamp_1",
            "right": "lamp_2",
            "up": "ceiling_fan",
            "down": "floor_lamp"
        }
        
        device = device_map.get(direction)
        if device:
            await self.toggle_device(device)
    
    async def toggle_device(self, device_id):
        """
        Toggle any smart home device
        """
        print(f"  → Toggling device: {device_id}")
        
        if self.config.get("home_assistant_url"):
            await self.call_home_assistant_service(
                "homeassistant.toggle",
                {"entity_id": device_id}
            )
        else:
            print(f"  ⚠ Device toggle simulation: {device_id}")
    
    async def call_uber(self):
        """
        Request an Uber ride via API
        """
        print("  → Calling Uber...")
        
        # Uber Rides API requires OAuth - simplified for demo
        uber_api_key = self.config.get("uber_api_key")
        
        if uber_api_key:
            # This is a simplified example - real implementation needs OAuth
            url = "https://api.uber.com/v1.2/requests"
            headers = {
                "Authorization": f"Bearer {uber_api_key}",
                "Content-Type": "application/json"
            }
            payload = {
                "product_id": "a1111c8c-c720-46c3-8534-2fcdd730040d",
                "start_latitude": 37.7749,  # Replace with actual location
                "start_longitude": -122.4194,
                "end_latitude": 37.7849,
                "end_longitude": -122.4294
            }
            
            async with aiohttp.ClientSession() as session:
                async with session.post(url, json=payload, headers=headers) as response:
                    if response.status == 202:
                        data = await response.json()
                        print(f"  ✓ Uber requested! ETA: {data.get('eta', 'unknown')} min")
                    else:
                        print(f"  ✗ Uber request failed: {response.status}")
        else:
            print("  ⚠ Uber API not configured (simulation)")
            print("  ✓ [DEMO] Uber would be called to current location")
    
    async def unlock_door(self):
        """
        Unlock smart door lock
        """
        print("  → Unlocking door...")
        
        if self.config.get("home_assistant_url"):
            await self.call_home_assistant_service(
                "lock.unlock",
                {"entity_id": "lock.front_door"}
            )
        else:
            print("  ⚠ Smart lock not configured (simulation)")
            print("  ✓ [DEMO] Front door unlocked")
    
    async def unlock_phone(self):
        """
        Send unlock signal to phone
        """
        print("  → Unlocking phone...")
        
        # This would typically use platform-specific APIs:
        # - iOS: Shortcuts app webhook
        # - Android: Tasker/Automate with HTTP request trigger
        
        # Example: Send HTTP request to phone's local webhook
        phone_webhook = "http://192.168.1.50:8765/unlock"
        
        try:
            async with aiohttp.ClientSession() as session:
                async with session.post(phone_webhook, timeout=2) as response:
                    if response.status == 200:
                        print("  ✓ Phone unlocked")
                    else:
                        print(f"  ✗ Failed to unlock phone: {response.status}")
        except Exception as e:
            print(f"  ⚠ Phone unlock not available (simulation)")
    
    async def media_control(self, action):
        """
        Control media playback (Spotify, etc.)
        """
        print(f"  → Media control: {action}")
        
        if self.config.get("home_assistant_url"):
            service_map = {
                "play": "media_player.media_play",
                "pause": "media_player.media_pause",
                "next": "media_player.media_next_track",
                "previous": "media_player.media_previous_track"
            }
            
            service = service_map.get(action)
            if service:
                await self.call_home_assistant_service(
                    service,
                    {"entity_id": "media_player.spotify"}
                )
        else:
            print(f"  ⚠ Media control simulation: {action}")
    
    async def call_home_assistant_service(self, service, entity_data):
        """
        Call Home Assistant service
        """
        url = f"{self.config['home_assistant_url']}/api/services/{service.replace('.', '/')}"
        headers = {
            "Authorization": f"Bearer {self.config['home_assistant_token']}",
            "Content-Type": "application/json"
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(url, json=entity_data, headers=headers) as response:
                if response.status == 200:
                    print(f"  ✓ Home Assistant: {service} executed")
                else:
                    print(f"  ✗ Home Assistant error: {response.status}")


class OpenMuscleBLEClient:
    """
    BLE client for OpenMuscle wristband
    """
    
    def __init__(self, device_name, action_controller):
        self.device_name = device_name
        self.action_controller = action_controller
        self.client = None
        self.connected = False
    
    async def scan_and_connect(self):
        """
        Scan for OpenMuscle device and connect
        """
        print(f"Scanning for device: {self.device_name}...")
        
        devices = await BleakScanner.discover(timeout=10.0)
        target_device = None
        
        for device in devices:
            if device.name and self.device_name in device.name:
                target_device = device
                break
        
        if not target_device:
            print(f"✗ Device '{self.device_name}' not found")
            print("\nAvailable devices:")
            for device in devices:
                print(f"  - {device.name} ({device.address})")
            return False
        
        print(f"✓ Found device: {target_device.name} ({target_device.address})")
        print(f"Connecting...")
        
        self.client = BleakClient(target_device.address)
        
        try:
            await self.client.connect()
            self.connected = True
            print(f"✓ Connected to {target_device.name}")
            
            # Subscribe to gesture notifications
            await self.client.start_notify(GESTURE_CHAR_UUID, self.gesture_notification_handler)
            print("✓ Subscribed to gesture notifications\n")
            
            return True
            
        except Exception as e:
            print(f"✗ Connection failed: {e}")
            return False
    
    def gesture_notification_handler(self, sender, data):
        """
        Handle incoming gesture notifications from wristband
        """
        try:
            # Decode JSON data
            gesture_json = data.decode('utf-8')
            gesture_data = json.loads(gesture_json)
            
            # Handle gesture asynchronously
            asyncio.create_task(self.action_controller.handle_gesture(gesture_data))
            
        except Exception as e:
            print(f"Error processing gesture data: {e}")
    
    async def send_command(self, command_data):
        """
        Send command to wristband
        """
        if not self.connected:
            return False
        
        command_json = json.dumps(command_data)
        await self.client.write_gatt_char(COMMAND_CHAR_UUID, command_json.encode('utf-8'))
        print(f"Sent command: {command_json}")
        return True
    
    async def disconnect(self):
        """
        Disconnect from device
        """
        if self.client and self.connected:
            await self.client.disconnect()
            self.connected = False
            print("\n✓ Disconnected from device")


async def main():
    parser = argparse.ArgumentParser(description='OpenMuscle BLE Receiver')
    parser.add_argument('--device', type=str, default='OpenMuscle-FlexGrid',
                       help='Device name to connect to')
    parser.add_argument('--config', type=str, default=None,
                       help='Path to JSON config file')
    
    args = parser.parse_args()
    
    # Load config from file if provided
    config = CONFIG
    if args.config:
        with open(args.config, 'r') as f:
            config.update(json.load(f))
    
    # Initialize controller
    action_controller = GestureActionController(config)
    
    # Create BLE client
    ble_client = OpenMuscleBLEClient(args.device, action_controller)
    
    # Connect to device
    if not await ble_client.scan_and_connect():
        sys.exit(1)
    
    print("=== Gesture Control Active ===")
    print("Waiting for gestures...")
    print("Press Ctrl+C to exit\n")
    
    try:
        # Keep running until interrupted
        while True:
            await asyncio.sleep(1)
            
            # Could add periodic health checks here
            if not ble_client.connected:
                print("⚠ Connection lost, attempting to reconnect...")
                await ble_client.scan_and_connect()
    
    except KeyboardInterrupt:
        print("\n\nShutting down...")
    
    finally:
        await ble_client.disconnect()


if __name__ == '__main__':
    asyncio.run(main())
