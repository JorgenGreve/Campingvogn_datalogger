import os
import subprocess

def before_upload(source, target, env):
    print("🔁 Closing Tera Term before upload...")
    try:
        # Brug Windows taskkill til at lukke alle instanser af Tera Term
        subprocess.run(["taskkill", "/f", "/im", "ttermpro.exe"], check=True)
    except subprocess.CalledProcessError as e:
        print("⚠️ Could not close Tera Term:", e)

Import("env")
env.AddPreAction("upload", before_upload)
