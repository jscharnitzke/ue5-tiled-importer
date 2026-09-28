# Tiled Importer

Import `.tsx` Tile Set and `.tmx` Tile Map files from Tiled Map Editor into Unreal Engine 5 natively. Necessary Textures and Paper2D Tile Sets and Tile Maps are created automatically during the import process.

## Installation

1. Clone this project or download it and extract the code directly into `<your-project>/Plugins`.
2. If your project is already open in Unreal, refresh your Visual Studio project.
3. Navigate to `Edit` > `Plugins`, find the `TiledImporter` plugin, and enable it.
4. Restart Unreal Engine.
5. If you see a message asking you to rebuild the `TiledInterchange` module, click `Yes`.

That's it! You can now import Tile Map and Tile Set files from Tiled Map Editor by dragging and dropping into the Content Browser or through the Import wizard. The plugin registers its import pipelines automatically; to customize them, create your own pipeline assets and assign them in Project Settings under `Engine` > `Interchange` > `Content Import Settings` > `Pipeline Stacks` > `Assets` > `Per Translator Pipelines`.

## Limitations

Support is currently limited to specific Tile Map settings and Tile Set collision objects.

**Tile Map Types**

- Hexagonal
- Isometric
- Orthogonal

**Tile Map settings**

- `Right Down` render order

**Tile Set collision objects**

- Rectangle

## Future Development

Future releases will add support for:

- Other render orders
- Polygon and circle collision objects
