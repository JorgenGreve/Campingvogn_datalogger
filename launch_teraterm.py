import os
import subprocess

def after_upload(source, target, env):
    # Find COM-port fra miljøvariabler (eller sæt manuelt fallback)
    port = env.get("UPLOAD_PORT") or env.get("MONITOR_PORT") or "COM5"
    baudrate = "115200"

    # Tilpas denne sti til hvor du har installeret Tera Term
    teraterm_path = r"C:\Program Files (x86)\teraterm5\ttermpro.exe"

    if os.path.exists(teraterm_path):
        subprocess.Popen([teraterm_path, port, "/BAUD=" + baudrate])
    else:
        print("⚠️ Tera Term executable not found at:", teraterm_path)

# Denne linje er vigtig: den skal stå **udenfor** funktionen ovenfor
Import("env")
env.AddPostAction("upload", after_upload)
