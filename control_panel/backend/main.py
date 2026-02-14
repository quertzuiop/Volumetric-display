from fastapi import FastAPI
from starlette.responses import FileResponse 
from pydantic import BaseModel
from contextlib import asynccontextmanager

import subprocess

import time

from shm import Shm

def print_nice(x): print(f"[Control panel] {x}")
    
DRIVER_BIN = "../../driver/build/main"
SPEED_REGULATOR_BIN = "../../speedRegulator/build/main"

class Key_stroke(BaseModel):
    key_code: str
    timestamp: int

class Key_strokes(BaseModel):
    keys: list[Key_stroke]

shm = Shm("vdshm")
shm.create()
processes = []

@asynccontextmanager
async def lifespan(app: FastAPI):
    global processes
    
    print_nice("Launching driver")
    try:
        p_driver = subprocess.Popen([DRIVER_BIN])
        processes.append(p_driver)
    except FileNotFoundError:
        print_nice(f"Error: Could not find {SPEED_REGULATOR_BIN}")
    
    time.sleep(0.5)
    print_nice("Launching speed regulator")
    
    try:
        p_speed = subprocess.Popen([SPEED_REGULATOR_BIN])
        processes.append(p_speed)
    except FileNotFoundError:
        print_nice(f"Error: Could not find {SPEED_REGULATOR_BIN}")
    
    time.sleep(0.5)
    
    yield
    
    shm.close()

app = FastAPI(lifespan = lifespan)
pressed_keys: list[Key_stroke]

@app.get("/")
async def read_index():
    return FileResponse('../index.html')

@app.get("/keystrokes/")
async def get_pressed_keys():
    return pressed_keys

@app.put("/keystrokes/")
async def set_pressed_keys(new: Key_strokes):
    print_nice("new keys", new)
    pressed_keys = new
    shm.write_keys(new.keys)
