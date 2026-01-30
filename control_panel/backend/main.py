from fastapi import FastAPI
from starlette.responses import FileResponse 
from pydantic import BaseModel
from shm import Shm

class Key_stroke(BaseModel):
    key_code: str
    timestamp: int

class Key_strokes(BaseModel):
    keys: list[Key_stroke]

shm = Shm("vdshm")
shm.create()

app = FastAPI()
pressed_keys: list[Key_stroke]

@app.get("/")
async def read_index():
    return FileResponse('../index.html')

@app.get("/keystrokes/")
async def get_pressed_keys():
    return pressed_keys

@app.put("/keystrokes/")
async def set_pressed_keys(new: Key_strokes):
    print("new keys", new)
    pressed_keys = new
    shm.write_keys(new.keys)