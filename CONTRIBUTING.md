# contributing

The project may seem a bit daunting at first, however please note that:

- As this is not a huge/widely-known/company-sponsored big project, there are no strong requirements for newcomers. Path of Exile is a very complex game and I won't punish people for the lack of knowledge.
- You don't need to have programming skills to contribute to the project. A worthy contributor can also help by:
  - reporting bugs
  - suggesting ideas
  - improving documentation
  - opening issues (question-only issues welcome)
  - recommending the tool for other players (if appropriate)

Interestingly, a lot of project's work does not involve programming:

- testing filters in-game (personally I recommend Azurite Mine as this is an area that both has stash and allows to drop items)
- language design/research, experiments
- incorporating game information into the project (eg datamining)

So far this project did not use any external programming tools or services and currently there are no plans to do so. Everything is expected to move through git and GitHub's website features (most notably issues) plus contact information (stated in main README).

## pull requests

Generally, stick to the `develop` branch. Use others if necessary. If in doubt, ask.

It is allowed to force push on non-master branches. I try not to overwrite history on develop. Force-pushing PRs is fine because I will generally squash commits that are fixes to previous commits.

## programming

For any related information, see files in `programmer` directory in the documentation.

## licensing

By submitting any work to the repository you agree for this work to be licensed under GNU GPL 3.0. The project license may change in the future to MPL if the project creator finds it useful for some use cases (most notably better interoperation with other software, including different programming languages).

## special thanks

To:

- https://github.com/aquach/poe-css for the idea of the project. FS eventually went a different route with the grammar but some fundamentals stay.
- `Alekhoff` for being the first power-user of the tool and offering some suggestions.
- `/u/Asymat` for FilterBlade help/discussions and long discussions on Discord.
- `/u/poorFishwife` for list of possible rare Fishing Rod names.
- `csiqueira` for help with Emscripten porting and web tech.
