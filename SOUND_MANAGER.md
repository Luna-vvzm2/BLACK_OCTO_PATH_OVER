# Sound.csv integration

Place `Sound.csv` and WAV files in `assets/SoundAssets`. A CSV `FilePath` such as `SoundAssets/SwordSwing.wav` is resolved from `assets`, so it reaches `assets/SoundAssets/SwordSwing.wav`. The CSV can also be found when the game is started from its build output folder.

`Game::Init()` reads `assets/SoundAssets/Sound.csv` and loads scope 0. A missing/invalid CSV or missing WAV is reported in the debug console; the game continues without the affected CSV sound. Scope 0 is shared by all scenes; stage 1, 2, and 3 use scopes 2, 3, and 4. Scope 1 is currently unused. Changing a stage or scene unloads sounds outside the new scope.

Create a pathless `SoundComponent` in an actor, then call the CSV `SoundId`:

```cpp
auto* sound = AddComponent<SoundComponent>();
if (sound) sound->Play("SwordSwing");
// sound->Stop();               // last ID played through this component
// sound->Stop("SwordSwing");  // explicit ID
```

Use `SetCategory(SoundCategory::Bgm)` or `SetCategory(SoundCategory::Voice)` before `Play(id)` to apply the corresponding settings volume. The default category is SE. CSV `Volume` and `Pan` apply as well. `SourceStart` and `SourceEnd` are 60-fps frame counts; the manager also accepts `hours:minutes:seconds:frames`. `Loop` comes from CSV, and `LoopFadeFrames` crossfades the loop boundary. Each ID currently has one playback instance, so replaying an ID restarts that instance.

The old hard-coded `assets/sounds/bgm.wav` stage playback was removed. No stage BGM starts automatically until the final CSV's BGM IDs are known and the appropriate `Play(id)` calls are added. Other existing file-path sounds remain unchanged.
