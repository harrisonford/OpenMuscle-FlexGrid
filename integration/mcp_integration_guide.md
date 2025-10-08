# MCP (Model Context Protocol) Integration Guide

## What is MCP?

Model Context Protocol (MCP) is a new standard by Anthropic that enables AI models (like ChatGPT, Claude) to securely interact with external tools, data sources, and services through a standardized interface.

**Why it's perfect for your project:**
- AI can interpret gesture context (e.g., "user is pointing at living room")
- Dynamic decision making (e.g., which light to control based on time/location)
- Natural language understanding of intent
- Orchestration of complex multi-step actions

---

## Architecture Overview

```
┌─────────────────┐
│   Wristband     │ → Detects gesture "point_up"
│   (ESP32-S3)    │ → Reads IMU: pitch=45°, roll=10°
└────────┬────────┘
         │ BLE
         ↓
┌─────────────────┐
│  Bridge Device  │ → Receives: {"gesture": "point_up", "imu": {...}}
│  (Phone/Pi)     │ → Adds context: location, time, nearby devices
└────────┬────────┘
         │ HTTPS
         ↓
┌─────────────────┐
│   MCP Server    │ → ChatGPT/Claude with MCP tools
│  (Cloud/Local)  │ → Interprets: "User wants to control ceiling light"
└────────┬────────┘
         │ API Calls
         ↓
┌─────────────────┐
│  Device APIs    │ → Executes: Turn on Philips Hue Light #1
│  (Hue, Uber,    │
│   SmartThings)  │
└─────────────────┘
```

---

## MCP Implementation Options

### Option 1: ChatGPT with Function Calling (Recommended for Hackathon)

**Pros:**
- Easy to implement
- Well-documented
- No MCP server setup needed
- Can use official OpenAI SDK

**Cons:**
- Not true MCP (uses function calling instead)
- Requires internet connection

**Implementation:**

```python
import openai
from openai import OpenAI

client = OpenAI(api_key="your-api-key")

# Define available tools (gestures → actions)
tools = [
    {
        "type": "function",
        "function": {
            "name": "control_light",
            "description": "Control a smart light based on user gesture and direction",
            "parameters": {
                "type": "object",
                "properties": {
                    "device_id": {
                        "type": "string",
                        "description": "ID of the light to control"
                    },
                    "action": {
                        "type": "string",
                        "enum": ["on", "off", "toggle", "dim", "brighten"],
                        "description": "Action to perform"
                    },
                    "brightness": {
                        "type": "integer",
                        "description": "Brightness level 0-100 (if dimming/brightening)"
                    }
                },
                "required": ["device_id", "action"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "call_ride",
            "description": "Request an Uber/Lyft ride to a destination",
            "parameters": {
                "type": "object",
                "properties": {
                    "destination": {
                        "type": "string",
                        "enum": ["home", "work", "custom"],
                        "description": "Destination preset"
                    },
                    "service": {
                        "type": "string",
                        "enum": ["uber", "lyft"],
                        "description": "Ride service to use"
                    }
                },
                "required": ["destination"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "unlock_device",
            "description": "Unlock a smart lock or phone",
            "parameters": {
                "type": "object",
                "properties": {
                    "device_type": {
                        "type": "string",
                        "enum": ["phone", "door", "car"],
                        "description": "Type of device to unlock"
                    }
                },
                "required": ["device_type"]
            }
        }
    }
]

async def interpret_gesture_with_ai(gesture_data, context):
    """
    Send gesture + context to ChatGPT, get back action to perform
    """
    # Build context-aware prompt
    messages = [
        {
            "role": "system",
            "content": """You are a gesture interpretation system for a smart wristband.
            Users perform gestures to control devices silently. Based on the gesture,
            IMU orientation, time, location, and nearby devices, determine which action
            to execute. Be specific about which device to control."""
        },
        {
            "role": "user",
            "content": f"""
            Gesture detected: {gesture_data['gesture']}
            Confidence: {gesture_data['confidence']}
            IMU orientation:
              - Pitch: {gesture_data['imu']['pitch']}° (positive = pointing up)
              - Roll: {gesture_data['imu']['roll']}° (positive = tilting right)
            
            Context:
              - Location: {context['location']}
              - Time: {context['time']}
              - Nearby devices: {', '.join(context['nearby_devices'])}
              - Previous action: {context.get('last_action', 'none')}
            
            What action should be performed?
            """
        }
    ]
    
    # Call ChatGPT with function calling
    response = client.chat.completions.create(
        model="gpt-4-turbo-preview",
        messages=messages,
        tools=tools,
        tool_choice="auto"
    )
    
    # Extract function call
    message = response.choices[0].message
    
    if message.tool_calls:
        tool_call = message.tool_calls[0]
        function_name = tool_call.function.name
        function_args = json.loads(tool_call.function.arguments)
        
        print(f"AI decided: {function_name}({function_args})")
        
        return {
            "action": function_name,
            "parameters": function_args,
            "reasoning": message.content
        }
    else:
        return {
            "action": "none",
            "reasoning": message.content
        }

# Example usage
async def handle_gesture(gesture_data):
    context = {
        "location": "living_room",
        "time": "19:30",
        "nearby_devices": ["ceiling_light", "table_lamp", "tv", "door_lock"],
        "last_action": "turned_on_tv"
    }
    
    action = await interpret_gesture_with_ai(gesture_data, context)
    
    # Execute the action
    if action["action"] == "control_light":
        await control_light(**action["parameters"])
    elif action["action"] == "call_ride":
        await call_uber(**action["parameters"])
    elif action["action"] == "unlock_device":
        await unlock_device(**action["parameters"])
```

---

### Option 2: Anthropic Claude with MCP (More Advanced)

**Pros:**
- True MCP implementation
- Better context understanding
- More flexible tool definitions

**Cons:**
- Newer, less documentation
- Requires MCP server setup

**Implementation:**

```python
import anthropic

client = anthropic.Anthropic(api_key="your-api-key")

# Define MCP tools
mcp_tools = [
    {
        "name": "control_smart_device",
        "description": "Control any smart home device based on gesture and context",
        "input_schema": {
            "type": "object",
            "properties": {
                "device_id": {"type": "string"},
                "action": {"type": "string"},
                "parameters": {"type": "object"}
            },
            "required": ["device_id", "action"]
        }
    }
]

async def interpret_with_claude(gesture_data, context):
    message = client.messages.create(
        model="claude-3-5-sonnet-20241022",
        max_tokens=1024,
        tools=mcp_tools,
        messages=[
            {
                "role": "user",
                "content": f"""
                User performed gesture: {gesture_data['gesture']}
                Context: {json.dumps(context, indent=2)}
                
                Determine which device to control and what action to take.
                """
            }
        ]
    )
    
    # Process tool use
    for block in message.content:
        if block.type == "tool_use":
            return {
                "tool": block.name,
                "input": block.input
            }
```

---

### Option 3: Local MCP Server (Best for Privacy)

**Pros:**
- No cloud dependency
- Faster response time
- Works offline
- Complete privacy

**Cons:**
- More setup required
- Need local LLM (Llama, Mistral)

**Setup:**

```bash
# Install MCP server
pip install mcp

# Create MCP server
mcp init gesture-control-server

# Define tools in server.py
```

---

## Integration with BLE Receiver

Update the `ble_receiver.py` to use MCP:

```python
from openai import OpenAI
import json

class AIGestureInterpreter:
    def __init__(self, api_key):
        self.client = OpenAI(api_key=api_key)
        self.context = self.load_context()
    
    def load_context(self):
        """
        Load current context: location, nearby devices, time, etc.
        In a real app, this would query device registries, location services, etc.
        """
        return {
            "location": "living_room",
            "nearby_devices": [
                {"id": "light_1", "name": "Ceiling Light", "type": "light", "state": "off"},
                {"id": "light_2", "name": "Table Lamp", "type": "light", "state": "on"},
                {"id": "tv_1", "name": "Samsung TV", "type": "tv", "state": "on"},
                {"id": "lock_1", "name": "Front Door", "type": "lock", "state": "locked"}
            ],
            "time_of_day": "evening",
            "user_preferences": {
                "default_uber_destination": "home",
                "preferred_brightness": 80
            }
        }
    
    async def interpret_and_execute(self, gesture_data):
        """
        Use AI to interpret gesture in context and execute appropriate action
        """
        # Build prompt with full context
        context_str = json.dumps(self.context, indent=2)
        
        messages = [
            {
                "role": "system",
                "content": """You are an intelligent gesture interpreter for a smart wristband.
                Analyze the gesture, IMU data, and environmental context to determine the
                most appropriate action. Consider spatial orientation, time of day, and
                user preferences."""
            },
            {
                "role": "user",
                "content": f"""
                Gesture: {gesture_data['gesture']}
                Confidence: {gesture_data.get('confidence', 1.0)}
                IMU Data:
                  Pitch: {gesture_data.get('imu', {}).get('pitch', 0)}°
                  Roll: {gesture_data.get('imu', {}).get('roll', 0)}°
                  
                Environment Context:
                {context_str}
                
                What action should I take?
                """
            }
        ]
        
        # Define available functions
        tools = self.get_tool_definitions()
        
        # Call AI
        response = self.client.chat.completions.create(
            model="gpt-4-turbo-preview",
            messages=messages,
            tools=tools,
            tool_choice="auto"
        )
        
        # Execute returned function
        return await self.execute_tool_call(response.choices[0].message)
    
    def get_tool_definitions(self):
        """Define all available tools/functions"""
        # ... (use tools definition from above)
        pass
    
    async def execute_tool_call(self, message):
        """Execute the function call returned by AI"""
        if not message.tool_calls:
            print(f"AI: {message.content}")
            return
        
        for tool_call in message.tool_calls:
            function_name = tool_call.function.name
            function_args = json.loads(tool_call.function.arguments)
            
            print(f"\n🤖 AI Decision: {function_name}")
            print(f"   Parameters: {function_args}")
            
            # Route to appropriate handler
            if function_name == "control_light":
                await self.control_light(**function_args)
            elif function_name == "call_ride":
                await self.call_ride(**function_args)
            # ... etc
```

---

## Context Building Strategies

To make AI decisions smarter, build rich context:

### 1. Device Registry
```python
class DeviceRegistry:
    def __init__(self):
        self.devices = {}
    
    def register_device(self, device):
        self.devices[device['id']] = device
    
    def get_devices_by_location(self, location):
        return [d for d in self.devices.values() 
                if d.get('location') == location]
    
    def get_devices_by_type(self, device_type):
        return [d for d in self.devices.values() 
                if d.get('type') == device_type]
```

### 2. Spatial Mapping
```python
class SpatialMapper:
    """Map IMU orientation to physical devices"""
    
    def __init__(self):
        self.device_positions = {
            "light_1": {"azimuth": 0, "elevation": 45},    # Ceiling, north
            "light_2": {"azimuth": 90, "elevation": 0},    # Table, east
            "tv_1": {"azimuth": 180, "elevation": 0},      # Wall, south
        }
    
    def get_pointed_device(self, pitch, roll, yaw):
        """Determine which device user is pointing at"""
        # Convert IMU to azimuth/elevation
        azimuth = yaw
        elevation = pitch
        
        # Find closest device
        min_distance = float('inf')
        closest_device = None
        
        for device_id, pos in self.device_positions.items():
            distance = self.angular_distance(
                azimuth, elevation,
                pos['azimuth'], pos['elevation']
            )
            if distance < min_distance:
                min_distance = distance
                closest_device = device_id
        
        if min_distance < 30:  # Within 30 degrees
            return closest_device
        return None
    
    def angular_distance(self, az1, el1, az2, el2):
        """Calculate angular distance between two directions"""
        import math
        az_diff = abs(az1 - az2)
        el_diff = abs(el1 - el2)
        return math.sqrt(az_diff**2 + el_diff**2)
```

### 3. User Preferences
```python
class UserPreferences:
    def __init__(self, user_id):
        self.user_id = user_id
        self.preferences = self.load_from_db()
    
    def load_from_db(self):
        return {
            "uber_default_destination": "home",
            "uber_preferred_car_type": "uberx",
            "light_default_brightness": 80,
            "smart_lock_confirmation": True,  # Require confirmation before unlocking
            "gesture_sensitivity": "medium"
        }
```

---

## Complete Example: MCP-Powered Gesture System

```python
#!/usr/bin/env python3
"""
Complete MCP-powered gesture control system
"""

import asyncio
from openai import OpenAI
from ble_receiver import OpenMuscleBLEClient, GestureActionController

class MCPGestureController(GestureActionController):
    """
    Enhanced gesture controller with AI interpretation
    """
    
    def __init__(self, config):
        super().__init__(config)
        self.ai_client = OpenAI(api_key=config['openai_api_key'])
        self.device_registry = DeviceRegistry()
        self.spatial_mapper = SpatialMapper()
        self.user_prefs = UserPreferences("user_123")
        
    async def handle_gesture(self, gesture_data):
        """
        Override base handler to use AI interpretation
        """
        # Build context
        context = self.build_context(gesture_data)
        
        # Ask AI what to do
        action = await self.interpret_gesture_with_ai(gesture_data, context)
        
        # Execute action
        await self.execute_action(action)
    
    def build_context(self, gesture_data):
        """Build rich context for AI"""
        imu = gesture_data.get('imu', {})
        
        # Determine which device is being pointed at
        pointed_device = self.spatial_mapper.get_pointed_device(
            imu.get('pitch', 0),
            imu.get('roll', 0),
            imu.get('yaw', 0)
        )
        
        return {
            "pointed_device": pointed_device,
            "nearby_devices": self.device_registry.get_devices_by_location("living_room"),
            "time": datetime.now().strftime("%H:%M"),
            "user_preferences": self.user_prefs.preferences
        }
    
    async def interpret_gesture_with_ai(self, gesture_data, context):
        """Use OpenAI to interpret gesture"""
        # ... (implementation from above)
        pass

# Run the enhanced system
async def main():
    config = {
        "openai_api_key": "your-key-here",
        # ... other config
    }
    
    controller = MCPGestureController(config)
    ble_client = OpenMuscleBLEClient("OpenMuscle-FlexGrid", controller)
    
    await ble_client.scan_and_connect()
    
    while True:
        await asyncio.sleep(1)

if __name__ == '__main__':
    asyncio.run(main())
```

---

## Testing MCP Integration (Without Wristband)

Create a test script to simulate gestures:

```python
# test_mcp_gestures.py
import asyncio
from mcp_gesture_controller import MCPGestureController

async def test():
    config = {"openai_api_key": "your-key"}
    controller = MCPGestureController(config)
    
    # Simulate various gestures
    test_gestures = [
        {
            "gesture": "point_up",
            "confidence": 0.95,
            "imu": {"pitch": 45, "roll": 0, "yaw": 0}
        },
        {
            "gesture": "fist_close",
            "confidence": 0.92,
            "imu": {"pitch": 0, "roll": 0, "yaw": 0}
        },
        {
            "gesture": "twist_cw",
            "confidence": 0.88,
            "imu": {"pitch": 0, "roll": 0, "yaw": 90}
        }
    ]
    
    for gesture in test_gestures:
        print(f"\n{'='*50}")
        print(f"Testing: {gesture['gesture']}")
        print(f"{'='*50}")
        await controller.handle_gesture(gesture)
        await asyncio.sleep(2)

asyncio.run(test())
```

---

## API Keys & Setup

### OpenAI API
1. Sign up at https://platform.openai.com
2. Generate API key
3. Add to config: `"openai_api_key": "sk-..."`

### Anthropic API (Alternative)
1. Sign up at https://console.anthropic.com
2. Generate API key
3. Add to config: `"anthropic_api_key": "sk-ant-..."`

---

## Cost Considerations

**OpenAI API Costs (GPT-4 Turbo):**
- Input: $0.01 / 1K tokens
- Output: $0.03 / 1K tokens
- Typical gesture interpretation: ~500 tokens = $0.02 per gesture

**Optimization Tips:**
1. Cache context data (don't resend device list every time)
2. Use cheaper models for simple gestures (GPT-3.5)
3. Implement local fallbacks for common gestures
4. Batch multiple gestures if rapid-fire

---

## Next Steps

1. **Start Simple**: Use basic function calling with OpenAI
2. **Add Context**: Build device registry and spatial mapping
3. **Refine Prompts**: Improve AI decision accuracy
4. **Add Confirmation**: For sensitive actions (unlocking doors)
5. **Optimize**: Add caching, local fallbacks, smarter routing

---

## Demo Script for Hackathon

```python
"""
Impressive demo flow showing MCP capabilities
"""

async def hackathon_demo():
    print("=== OpenMuscle + MCP Demo ===\n")
    
    # Demo 1: Context-aware light control
    print("Demo 1: Point at ceiling light...")
    gesture = {"gesture": "point_up", "imu": {"pitch": 45}}
    # AI determines it's the ceiling light, turns it on
    
    # Demo 2: Smart Uber calling
    print("\nDemo 2: Fist gesture...")
    gesture = {"gesture": "fist_close"}
    # AI knows it's 5 PM on Friday → calls Uber to home
    
    # Demo 3: Spatial awareness
    print("\nDemo 3: Point left...")
    gesture = {"gesture": "point_left", "imu": {"roll": -45}}
    # AI calculates which device is to the left, controls it
    
    # Demo 4: Multi-step reasoning
    print("\nDemo 4: Twist gesture near door...")
    gesture = {"gesture": "twist_cw"}
    # AI knows: near door + twist = unlock
    # But checks: is it night? → adds security confirmation
```

This MCP integration makes your wristband truly intelligent! 🚀
