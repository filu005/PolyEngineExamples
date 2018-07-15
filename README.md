<h1> <img src="Media/PolyEngine_logo_black.png" width="128"> PolyEngineExamples </h1>

[![license](https://img.shields.io/github/license/KNTGPolygon/PolyEngineExamples.svg)](LICENSE)

Game examples for [**PolyEngine**](https://github.com/KNTGPolygon/PolyEngine) - study project developed by [**KNTG Polygon**](http://polygon.pw.edu.pl/), licensed under a permissive MIT license.  

## Running the examples
1. Clone PolyEngine repository and this repository to your computer
2. Make sure you have Python 3.6 installed
3. On *nix platforms install all missing dependancies (for Ubuntu 18.04 you can use Scripts/ubuntu18.04_install_dependencies.sh)
4. Go to `<EngineRepository>/PolyEngine/Scripts/`
5. Run `py ProjectTool.py -u <ExamplesRepository>/<GameName>`. This will create make/sln file in `<ExamplesRepository>/<GameName>/Build`.
6. Windows: Open `<GameName>.sln` and set project `<GameName>` as startup project. Compile and Run!
6. *nix: Go to make file location and run make. Go to `./<GameName>` folder and run `PolyStandalone`.

## Branches:
* master - main branch of the project, intended for stable, tested versions of the examples. Releases will be additionally tagged. Guaranteed to be rebase/force-push free.
* dev - branch intended for pull request integration and stability testing. Guaranteed to be rebase/force-push free.

Other branches can be removed, renamed or rebased anytime.
