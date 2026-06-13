"""
Generate 1942-themed pixel art assets using the PixelLab API.

Usage:
    $env:PIXELLAB_SECRET = "your-api-key-here"
    python tools/generate_assets.py

    # Generate only a specific category:
    python tools/generate_assets.py player
    python tools/generate_assets.py enemies
    python tools/generate_assets.py background
    python tools/generate_assets.py bullets
    python tools/generate_assets.py debris
    python tools/generate_assets.py explosions
    python tools/generate_assets.py hud

Run from the project root (the folder containing 'assets/').
Re-running is safe -- existing files are skipped.
Delete a file to regenerate it.
"""

import sys
import time
from pathlib import Path
from pixellab.client import PixelLabClient

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
ASSETS = Path("assets")

# ---------------------------------------------------------------------------
# Style defaults for aircraft sprites
# ---------------------------------------------------------------------------
AIRCRAFT_STYLE = dict(
    outline="single color black outline",
    shading="medium shading",
    detail="medium detail",
    view="high top-down",
    no_background=True,
)

FLAT_NO_BG = dict(
    outline="single color black outline",
    shading="flat shading",
    detail="low detail",
    no_background=True,
)

# ---------------------------------------------------------------------------
# Sprite manifest: (relative_path, width, height, prompt, extra_kwargs)
# ---------------------------------------------------------------------------

# --- Background (no transparency, no outline needed) ---
_BG_PROMPT = (
    "Pacific Ocean aerial view from high altitude, deep blue ocean water, "
    "small tropical island with palm trees and white sandy beach, scattered "
    "white clouds below the plane, green coral reefs, top-down aerial view, "
    "WWII era, pixel art tileable texture"
)
BACKGROUND = [
    ("Backgrounds/darkPurple.png", 400, 400, _BG_PROMPT,
     dict(outline="lineless", shading="basic shading", detail="medium detail",
          no_background=False, isometric=False)),
]

# --- Player: P-38 Lightning (128x128, heading north) ---
_P38_BASE = (
    "World War 2 American P-38 Lightning twin-boom fighter aircraft, "
    "pixel art, top-down aerial view, military olive drab green wings, "
    "bare aluminium fuselage, WWII Pacific theater, clean lines, "
    "heading north"
)
PLAYER = [
    ("PNG/playerShip1_orange.png", 128, 128,
     _P38_BASE,
     dict(**AIRCRAFT_STYLE, direction="north", seed=42)),

    ("PNG/Damage/playerShip1_damage1.png", 128, 128,
     _P38_BASE + ", light scorch marks on fuselage, small wisp of smoke from engine",
     dict(**AIRCRAFT_STYLE, direction="north", seed=43)),

    ("PNG/Damage/playerShip1_damage2.png", 128, 128,
     _P38_BASE + ", moderate battle damage, torn wing panel, smoke trail from one engine",
     dict(**AIRCRAFT_STYLE, direction="north", seed=44)),

    ("PNG/Damage/playerShip1_damage3.png", 128, 128,
     _P38_BASE + ", heavy battle damage, engine on fire, black smoke trailing, "
     "bullet holes across fuselage, one engine destroyed",
     dict(**AIRCRAFT_STYLE, direction="north", seed=45)),
]

# --- Enemies ---
_ZERO_BASE = (
    "WW2 Imperial Japanese Mitsubishi A6M Zero fighter aircraft, pixel art, "
    "top-down aerial view, heading south, dark grey and black camouflage, "
    "Japanese rising sun red circle roundel on wings, single engine propeller plane"
)
_KI43_BASE = (
    "WW2 Imperial Japanese Nakajima Ki-43 Hayabusa fighter aircraft, pixel art, "
    "top-down aerial view, heading south, red-orange and dark brown aggressive "
    "paint scheme, single propeller engine"
)
_KI61_BASE = (
    "WW2 Imperial Japanese Kawasaki Ki-61 Tony fighter aircraft, pixel art, "
    "top-down aerial view, heading south, blue-grey military camouflage, "
    "inline liquid-cooled engine, single propeller plane"
)
_BETTY_BASE = (
    "WW2 Imperial Japanese Mitsubishi G4M Betty medium bomber aircraft, pixel art, "
    "top-down aerial view, heading south, dark olive green camouflage, "
    "twin piston engines, wide cigar-shaped fuselage, long wingspan"
)
_UFO_PROMPT = (
    "WW2 large Imperial Japanese Kawanishi H8K flying boat aircraft, pixel art, "
    "top-down aerial view, heading south, dark green and grey hull, "
    "massive four-engine seaplane, wide high-mounted wings, large command aircraft"
)

ENEMIES = []
for i in range(1, 6):
    seed = i
    ENEMIES.append((f"PNG/Enemies/enemyBlack{i}.png", 96, 96,
                     _ZERO_BASE, dict(**AIRCRAFT_STYLE, direction="south", seed=seed)))
for i in range(1, 6):
    seed = i + 10
    ENEMIES.append((f"PNG/Enemies/enemyRed{i}.png", 96, 96,
                     _KI43_BASE, dict(**AIRCRAFT_STYLE, direction="south", seed=seed)))
for i in range(1, 6):
    seed = i + 20
    ENEMIES.append((f"PNG/Enemies/enemyBlue{i}.png", 96, 96,
                     _KI61_BASE, dict(**AIRCRAFT_STYLE, direction="south", seed=seed)))
for i in range(1, 6):
    seed = i + 30
    ENEMIES.append((f"PNG/Enemies/enemyGreen{i}.png", 96, 96,
                     _BETTY_BASE, dict(**AIRCRAFT_STYLE, direction="south", seed=seed)))

# UFO: large flying boat, wider than tall
ENEMIES.append(("PNG/ufoRed.png", 128, 96, _UFO_PROMPT,
                dict(**AIRCRAFT_STYLE, direction="south", seed=99)))

# --- Bullets ---
BULLETS = [
    ("PNG/Lasers/laserBlue01.png", 16, 64,
     "Bright yellow-white glowing tracer bullet streak, tall thin elongated shape, "
     "WWII anti-aircraft tracer round, glowing core, pixel art",
     dict(outline="lineless", shading="flat shading", detail="low detail",
          no_background=True, seed=1)),

    ("PNG/Lasers/laserRed01.png", 16, 64,
     "Bright red-orange glowing enemy tracer bullet streak, tall thin elongated shape, "
     "WWII Japanese tracer round, glowing core, pixel art",
     dict(outline="lineless", shading="flat shading", detail="low detail",
          no_background=True, seed=2)),
]

# --- Debris (replaces meteor sprites with WWII-themed falling objects) ---
DEBRIS = [
    ("PNG/Meteors/meteorBrown_big1.png", 80, 80,
     "WW2 aerial bomb falling, top-down view, dark grey metal cylindrical bomb, "
     "fins visible, pixel art",
     dict(**FLAT_NO_BG, seed=10)),

    ("PNG/Meteors/meteorBrown_big2.png", 80, 80,
     "WW2 burning aircraft debris falling, scattered metal scraps, small orange flames, "
     "top-down view, pixel art",
     dict(**FLAT_NO_BG, seed=11)),

    ("PNG/Meteors/meteorGrey_big1.png", 80, 80,
     "Anti-aircraft flak burst fragment, jagged grey metal shrapnel spinning, "
     "top-down view, pixel art",
     dict(**FLAT_NO_BG, seed=12)),

    ("PNG/Meteors/meteorGrey_big2.png", 80, 80,
     "Falling aircraft auxiliary fuel drop tank, grey oval cylinder, on fire, "
     "top-down view, pixel art",
     dict(**FLAT_NO_BG, seed=13)),
]

# --- Explosion frames (20 frames, 64x64 each) ---
_EXPLOSION_STAGES = [
    # (frame_range, prompt_description)
    (range(0, 3),   "tiny bright white flash point, initial detonation spark, "
                    "almost nothing visible yet, minimal pixels"),
    (range(3, 7),   "small yellow-orange fireball expanding, bright center, "
                    "small explosion beginning"),
    (range(7, 11),  "medium orange and red fireball, bright yellow core, "
                    "expanding explosion ball, some dark edges"),
    (range(11, 15), "large red-orange explosion fireball, dark smoke ring forming "
                    "around edges, billowing outward"),
    (range(15, 18), "explosion fading, large dark grey-black smoke cloud, "
                    "faint orange glow at center, expanding smoke"),
    (range(18, 20), "dissipating dark smoke cloud, mostly black and grey, "
                    "almost no flame visible, final frames of explosion"),
]

EXPLOSIONS = []
for frame_range, stage_desc in _EXPLOSION_STAGES:
    for i in frame_range:
        prompt = (
            f"Pixel art WWII aircraft explosion animation frame {i+1} of 20: "
            f"{stage_desc}. Transparent background, no background, "
            f"top-down view, retro arcade style."
        )
        EXPLOSIONS.append((f"PNG/Effects/fire{i:02d}.png", 64, 64, prompt,
                           dict(outline="lineless", shading="basic shading",
                                detail="medium detail", no_background=True, seed=i)))

# --- Terrain: boats / naval vessels drifting in open water (top-down) ---
# Note: the shipped sprites were generated via the PixelLab MCP "object" tool;
# these entries reproduce equivalent assets through the pixflux API path.
TERRAIN = [
    ("PNG/Terrain/boat_destroyer.png", 128, 128,
     "WWII Imperial Japanese Navy destroyer warship seen from directly above, "
     "top-down aerial bird's eye view, long narrow grey steel hull, gun turrets "
     "fore and aft, two smokestacks, pointed bow at top, white wake, naval vessel",
     dict(outline="single color black outline", shading="medium shading",
          detail="medium detail", view="high top-down", no_background=True, seed=201)),

    ("PNG/Terrain/boat_transport.png", 96, 96,
     "WWII Japanese Navy troop transport landing barge seen from directly above, "
     "top-down aerial bird's eye view, flat rectangular wooden and rusty-grey deck, "
     "blunt bow, small cargo boat in ocean",
     dict(outline="single color black outline", shading="medium shading",
          detail="medium detail", view="high top-down", no_background=True, seed=202)),

    ("PNG/Terrain/boat_gunboat.png", 64, 64,
     "WWII Imperial Japanese Navy small patrol gunboat seen from directly above, "
     "top-down aerial bird's eye view, compact grey armed motor boat, single gun "
     "turret, pointed bow, white wake",
     dict(outline="single color black outline", shading="medium shading",
          detail="medium detail", view="high top-down", no_background=True, seed=203)),
]

# --- HUD sprites ---
HUD = [
    ("PNG/UI/playerLife1_orange.png", 32, 32,
     "Small WW2 American P-38 Lightning fighter plane silhouette, pixel art, "
     "side view angled slightly top-down, solid olive green, minimal detail, "
     "life counter icon for retro arcade game HUD",
     dict(**FLAT_NO_BG, seed=1)),
]
for digit in range(10):
    HUD.append((f"PNG/UI/numeral{digit}.png", 16, 24,
                f"Pixel art bold military stencil numeral {digit}, "
                "white digit on transparent background, "
                "army stencil style, retro arcade HUD score font",
                dict(outline="lineless", shading="flat shading",
                     detail="low detail", no_background=True, seed=digit + 100)))

# ---------------------------------------------------------------------------
# Category registry
# ---------------------------------------------------------------------------
CATEGORIES = {
    "background": BACKGROUND,
    "player":     PLAYER,
    "enemies":    ENEMIES,
    "bullets":    BULLETS,
    "debris":     DEBRIS,
    "explosions": EXPLOSIONS,
    "terrain":    TERRAIN,
    "hud":        HUD,
}
ALL = [item for sprites in CATEGORIES.values() for item in sprites]

# ---------------------------------------------------------------------------
# Generation
# ---------------------------------------------------------------------------

def generate(client, manifest):
    total_cost = 0.0
    skipped = 0
    generated = 0

    for rel_path, w, h, prompt, kwargs in manifest:
        out = ASSETS / rel_path
        if out.exists():
            print(f"  skip    {rel_path}")
            skipped += 1
            continue

        out.parent.mkdir(parents=True, exist_ok=True)

        # Extract only params accepted by generate_image_pixflux
        accepted = {
            "outline", "shading", "detail", "view", "direction",
            "isometric", "no_background", "coverage_percentage",
            "init_image_strength", "seed", "text_guidance_scale",
        }
        clean_kwargs = {k: v for k, v in kwargs.items() if k in accepted}

        for attempt in range(3):
            try:
                resp = client.generate_image_pixflux(
                    description=prompt,
                    image_size={"width": w, "height": h},
                    **clean_kwargs,
                )
                break
            except Exception as exc:
                if attempt == 2:
                    print(f"  ERROR   {rel_path}: {exc}")
                    break
                print(f"  retry   {rel_path} (attempt {attempt + 1}): {exc}")
                time.sleep(2)
        else:
            continue

        img = resp.image.pil_image()
        img.save(str(out))
        total_cost += resp.usage.usd
        generated += 1
        print(f"  ${resp.usage.usd:.4f}  [{w}x{h}]  {rel_path}")

    print(f"\nDone: {generated} generated, {skipped} skipped")
    print(f"Total cost: ${total_cost:.4f}")


def main():
    filter_cat = sys.argv[1].lower() if len(sys.argv) > 1 else None

    if filter_cat and filter_cat not in CATEGORIES:
        print(f"Unknown category '{filter_cat}'. Choose from: {', '.join(CATEGORIES)}")
        sys.exit(1)

    manifest = CATEGORIES[filter_cat] if filter_cat else ALL
    print(f"Generating {len(manifest)} sprites "
          f"({'category: ' + filter_cat if filter_cat else 'all categories'}) ...")
    print(f"Output root: {ASSETS.resolve()}\n")

    client = PixelLabClient.from_env()

    # Show balance before starting
    try:
        from pixellab.get_balance import get_balance
        bal = get_balance(client)
        print(f"Account balance: ${bal.usd:.4f} USD\n")
    except Exception:
        pass

    generate(client, manifest)


if __name__ == "__main__":
    main()
