// ANDROID ASSET EXTRACTION IMPLEMENTATION
// Copies game data from APK assets to the app's internal storage.
//
// SDL_EnumerateDirectory uses POSIX opendir() on Android and therefore
// CANNOT enumerate APK asset paths.  Instead we keep a complete, explicit
// list of every game data file.  SDL_IOFromFile() with a relative path
// DOES read from the APK asset bundle on Android, so we use that for the
// actual byte-for-byte copy.

#ifdef __ANDROID__

#include "AndroidAssets.h"

#include <SDL3/SDL.h>
#include <android/log.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  "Bugdom2", __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "Bugdom2", __VA_ARGS__)

// Version file: if this file exists and contains our version, skip extraction.
// Bump this string whenever the Data/ directory contents change.
#define EXTRACT_VERSION_FILE  ".extract_version"
#define EXTRACT_VERSION       "4.0.1"

// -------------------------------------------------------------------------
// Complete list of all game data files, relative to the Data/ root.
// These are the exact paths that end up at the APK asset bundle root
// (because build.gradle.kts uses  assets.srcDirs("../../Data")).
// -------------------------------------------------------------------------
static const char *kAllDataFiles[] = {
    "Audio/Balsa/AntHillBoom.aiff",
    "Audio/Balsa/BalsaShoot.aiff",
    "Audio/Balsa/BombBoom.aiff",
    "Audio/Balsa/BombFall.aiff",
    "Audio/Balsa/DiveBomb.aiff",
    "Audio/Balsa/DragonFlyHit.aiff",
    "Audio/Balsa/FrogJump.aiff",
    "Audio/Balsa/PlaneHit.aiff",
    "Audio/Balsa/Propeller.aiff",
    "Audio/Balsa/SamAntHills1.aiff",
    "Audio/Balsa/SamAntHills2.aiff",
    "Audio/Balsa/SamAntHills3.aiff",
    "Audio/Bonus/CloverBonus.aiff",
    "Audio/Bonus/MouseBonus.aiff",
    "Audio/Closet/ChipClick.aiff",
    "Audio/Closet/MineBoom.aiff",
    "Audio/Closet/MothFlap.aiff",
    "Audio/Closet/SamComputer.aiff",
    "Audio/Closet/SamMoths.aiff",
    "Audio/Closet/SamRedClovers1.aiff",
    "Audio/Closet/SamRedClovers2.aiff",
    "Audio/Closet/Servo1.aiff",
    "Audio/Closet/Servo2.aiff",
    "Audio/Closet/SiliconDoorOpen.aiff",
    "Audio/Closet/Vacuum.aiff",
    "Audio/Closet/VacuumCrunch.aiff",
    "Audio/Fido/BoneHit.aiff",
    "Audio/Fido/SamGotFleas.aiff",
    "Audio/Fido/SamGotTicks.aiff",
    "Audio/Fido/SamHappyDog.aiff",
    "Audio/Fido/SamRemember.aiff",
    "Audio/Fido/TickDie.aiff",
    "Audio/Fido/TickSpit.aiff",
    "Audio/Fido/TickStep.aiff",
    "Audio/Fido/TickSuck.aiff",
    "Audio/Garbage/CanOpen.aiff",
    "Audio/Garbage/Propeller2.aiff",
    "Audio/Garbage/SamFlood1.aiff",
    "Audio/Garbage/SamFlood2.aiff",
    "Audio/Garbage/SamGlider.aiff",
    "Audio/Garbage/SamSoda.aiff",
    "Audio/Garbage/SodaSpray.aiff",
    "Audio/Garden/ChipStuckMouse.aiff",
    "Audio/Garden/EvilPlantShoot.aiff",
    "Audio/Garden/GnomeGotKicked.aiff",
    "Audio/Garden/GnomeStep.aiff",
    "Audio/Garden/SamBerries1.aiff",
    "Audio/Garden/SamBerries2.aiff",
    "Audio/Garden/SamBerries3.aiff",
    "Audio/Garden/SamFido.aiff",
    "Audio/Garden/SamFindShell1.aiff",
    "Audio/Garden/SamFindShell2.aiff",
    "Audio/Garden/SamFreeMice1.aiff",
    "Audio/Garden/SamFreeMice2.aiff",
    "Audio/Garden/SamFreeMice3.aiff",
    "Audio/Garden/SamPoolKey.aiff",
    "Audio/Garden/SamScarecrow1.aiff",
    "Audio/Garden/SamScarecrow2.aiff",
    "Audio/Garden/SamScarecrow3.aiff",
    "Audio/Garden/Sprinkler.aiff",
    "Audio/Garden/SquishBerry.aiff",
    "Audio/Main/AcornKicked.aiff",
    "Audio/Main/BottleCapBounce.aiff",
    "Audio/Main/BottleCrack.aiff",
    "Audio/Main/BottleShatter.aiff",
    "Audio/Main/BuddyBoom.aiff",
    "Audio/Main/BuddyBuzz.aiff",
    "Audio/Main/BuddyLaunch.aiff",
    "Audio/Main/BumbleRumble.aiff",
    "Audio/Main/ButterflyBoom.aiff",
    "Audio/Main/ChangeSelect.aiff",
    "Audio/Main/ChipCheckpoint1.aiff",
    "Audio/Main/ChipCheckpoint2.aiff",
    "Audio/Main/ChipMap1.aiff",
    "Audio/Main/ChipMap2.aiff",
    "Audio/Main/DoorCreak.aiff",
    "Audio/Main/DragonFlyBuzz.aiff",
    "Audio/Main/Firecracker.aiff",
    "Audio/Main/FlyGotKicked.aiff",
    "Audio/Main/FlyWalkBuzz.aiff",
    "Audio/Main/Footstep.aiff",
    "Audio/Main/GetPOW.aiff",
    "Audio/Main/GrenadeBoom.aiff",
    "Audio/Main/GrenadeThrow.aiff",
    "Audio/Main/Jump.aiff",
    "Audio/Main/MouseTrap.aiff",
    "Audio/Main/PlaneCrash.aiff",
    "Audio/Main/PopAcorn.aiff",
    "Audio/Main/PullTrap.aiff",
    "Audio/Main/Shield.aiff",
    "Audio/Main/SkipGlide.aiff",
    "Audio/Main/SkipKick.aiff",
    "Audio/Main/SkipLand.aiff",
    "Audio/Main/Smack.aiff",
    "Audio/Main/SnapTrap.aiff",
    "Audio/Main/Splash.aiff",
    "Audio/Main/ThrowBottleCap.aiff",
    "Audio/Music/BalsaSong.aiff",
    "Audio/Music/BonusSong.aiff",
    "Audio/Music/ClosetSong.aiff",
    "Audio/Music/FidoSong.aiff",
    "Audio/Music/GarbageSong.aiff",
    "Audio/Music/GardenSong.aiff",
    "Audio/Music/LoseSong.aiff",
    "Audio/Music/ParkSong.aiff",
    "Audio/Music/PlayroomSong.aiff",
    "Audio/Music/PlumbingSong.aiff",
    "Audio/Music/PoolSong.aiff",
    "Audio/Music/ThemeSong.aiff",
    "Audio/Music/TitleSong.aiff",
    "Audio/Music/WinSong.aiff",
    "Audio/Park/AntBite.aiff",
    "Audio/Park/FishFlop.aiff",
    "Audio/Park/FrogJump.aiff",
    "Audio/Park/SamBottleKey.aiff",
    "Audio/Park/SamFish1.aiff",
    "Audio/Park/SamFish2.aiff",
    "Audio/Park/SamFish3.aiff",
    "Audio/Park/SamFood1.aiff",
    "Audio/Park/SamFood2.aiff",
    "Audio/Park/SamFood3.aiff",
    "Audio/Park/SamHive1.aiff",
    "Audio/Park/SamHive2.aiff",
    "Audio/Park/SamHive3.aiff",
    "Audio/Park/SamHive4.aiff",
    "Audio/Park/TongueHit.aiff",
    "Audio/Park/TongueSwoosh.aiff",
    "Audio/Playroom/BowlingHit.aiff",
    "Audio/Playroom/ChipDoRace.aiff",
    "Audio/Playroom/ChipGo.aiff",
    "Audio/Playroom/ChipLostRace.aiff",
    "Audio/Playroom/ChipReady.aiff",
    "Audio/Playroom/ChipSamWinning.aiff",
    "Audio/Playroom/ChipSet.aiff",
    "Audio/Playroom/ChipWinning.aiff",
    "Audio/Playroom/ChipYouWon.aiff",
    "Audio/Playroom/KickMarble.aiff",
    "Audio/Playroom/LaserBoom.aiff",
    "Audio/Playroom/OttoFall.aiff",
    "Audio/Playroom/OttoMotor.aiff",
    "Audio/Playroom/OttoShoot.aiff",
    "Audio/Playroom/SamMarble1.aiff",
    "Audio/Playroom/SamMarble2.aiff",
    "Audio/Playroom/SamMarble3.aiff",
    "Audio/Playroom/SamPuzzle1.aiff",
    "Audio/Playroom/SamPuzzle2.aiff",
    "Audio/Playroom/SlotCar.aiff",
    "Audio/Plumbing/GutterWater.aiff",
    "Audio/Plumbing/HitLeaf.aiff",
    "Audio/Plumbing/HitNail.aiff",
    "Audio/Plumbing/HitPineCone.aiff",
    "Audio/Plumbing/HitSludge.aiff",
    "Audio/Plumbing/MetalScrape.aiff",
    "Audio/Plumbing/SamSewerIntro.aiff",
    "Audio/Title/FlyBuzz.aiff",
    "Audio/Title/FlySwatter.aiff",
    "Audio/Title/LogoBounce.aiff",
    "Audio/Title/LogoVanish.aiff",
    "Audio/Title/SmackDown.aiff",
    "Audio/Title/Stomp.aiff",
    "Models/Bonus.bg3d",
    "Models/Foliage.bg3d",
    "Models/Global.bg3d",
    "Models/HighScores.bg3d",
    "Models/Level10_Park.bg3d",
    "Models/Level1_Garden.bg3d",
    "Models/Level2_Sidewalk.bg3d",
    "Models/Level4_Plumbing.bg3d",
    "Models/Level5_Playroom.bg3d",
    "Models/Level6_Closet.bg3d",
    "Models/Level7_Gutter.bg3d",
    "Models/Level8_Garbage.bg3d",
    "Models/Level9_Balsa.bg3d",
    "Models/LevelIntro.bg3d",
    "Models/LoseScreen.bg3d",
    "Models/MainMenu.bg3d",
    "Models/Title.bg3d",
    "Models/WinScreen.bg3d",
    "Skeletons/Ant.bg3d",
    "Skeletons/Ant.skeleton.rsrc",
    "Skeletons/BuddyBug.bg3d",
    "Skeletons/BuddyBug.skeleton.rsrc",
    "Skeletons/BumbleBee.bg3d",
    "Skeletons/BumbleBee.skeleton.rsrc",
    "Skeletons/Checkpoint.bg3d",
    "Skeletons/Checkpoint.skeleton.rsrc",
    "Skeletons/Chipmunk.bg3d",
    "Skeletons/Chipmunk.skeleton.rsrc",
    "Skeletons/ComputerBug.bg3d",
    "Skeletons/ComputerBug.skeleton.rsrc",
    "Skeletons/DragonFly.bg3d",
    "Skeletons/DragonFly.skeleton.rsrc",
    "Skeletons/EvilPlant.bg3d",
    "Skeletons/EvilPlant.skeleton.rsrc",
    "Skeletons/Fish.bg3d",
    "Skeletons/Fish.skeleton.rsrc",
    "Skeletons/Flea.bg3d",
    "Skeletons/Flea.skeleton.rsrc",
    "Skeletons/Frog.bg3d",
    "Skeletons/Frog.skeleton.rsrc",
    "Skeletons/Gnome.bg3d",
    "Skeletons/Gnome.skeleton.rsrc",
    "Skeletons/Grasshopper.bg3d",
    "Skeletons/HoboBag.bg3d",
    "Skeletons/HoboBag.skeleton.rsrc",
    "Skeletons/HouseFly.bg3d",
    "Skeletons/HouseFly.skeleton.rsrc",
    "Skeletons/Moth.bg3d",
    "Skeletons/Moth.skeleton.rsrc",
    "Skeletons/Mouse.bg3d",
    "Skeletons/Mouse.skeleton.rsrc",
    "Skeletons/MouseTrap.bg3d",
    "Skeletons/MouseTrap.skeleton.rsrc",
    "Skeletons/OttoToy.bg3d",
    "Skeletons/OttoToy.skeleton.rsrc",
    "Skeletons/Roach.bg3d",
    "Skeletons/Roach.skeleton.rsrc",
    "Skeletons/Skip_Explore.skeleton.rsrc",
    "Skeletons/Skip_Title.skeleton.rsrc",
    "Skeletons/Skip_Tunnel.skeleton.rsrc",
    "Skeletons/Snail.bg3d",
    "Skeletons/Snail.skeleton.rsrc",
    "Skeletons/SnakeHead.bg3d",
    "Skeletons/SnakeHead.skeleton.rsrc",
    "Skeletons/Soldier.bg3d",
    "Skeletons/Soldier.skeleton.rsrc",
    "Skeletons/Tick.bg3d",
    "Skeletons/Tick.skeleton.rsrc",
    "Sprites/Bonus/000.tga",
    "Sprites/Bonus/001.tga",
    "Sprites/Bonus/002.tga",
    "Sprites/Bonus/003.tga",
    "Sprites/Bonus/004.tga",
    "Sprites/Bonus/005.tga",
    "Sprites/Bonus/006.tga",
    "Sprites/Bonus/007.tga",
    "Sprites/Bonus/008.tga",
    "Sprites/Bonus/009.tga",
    "Sprites/Bonus/010.tga",
    "Sprites/Bonus/011.tga",
    "Sprites/Bonus/012.tga",
    "Sprites/Bonus/013.tga",
    "Sprites/Bonus/014.tga",
    "Sprites/Font/font.tga",
    "Sprites/Font/font.txt",
    "Sprites/Global/000.tga",
    "Sprites/Global/001.tga",
    "Sprites/Global/002.tga",
    "Sprites/Global/003.tga",
    "Sprites/Global/004.tga",
    "Sprites/Global/005.tga",
    "Sprites/Global/006.tga",
    "Sprites/Global/007.tga",
    "Sprites/Global/008.tga",
    "Sprites/Global/009.tga",
    "Sprites/Global/010.tga",
    "Sprites/Global/011.tga",
    "Sprites/Global/012.tga",
    "Sprites/Global/013.tga",
    "Sprites/Global/014.tga",
    "Sprites/Infobar/000.tga",
    "Sprites/Infobar/001.tga",
    "Sprites/Infobar/002.tga",
    "Sprites/Infobar/003.tga",
    "Sprites/Infobar/004.tga",
    "Sprites/Infobar/005.tga",
    "Sprites/Infobar/006.tga",
    "Sprites/Infobar/007.tga",
    "Sprites/Infobar/008.tga",
    "Sprites/Infobar/009.tga",
    "Sprites/Infobar/010.tga",
    "Sprites/Infobar/011.tga",
    "Sprites/Infobar/012.tga",
    "Sprites/Infobar/013.tga",
    "Sprites/Infobar/014.tga",
    "Sprites/Infobar/015.tga",
    "Sprites/Infobar/016.tga",
    "Sprites/Infobar/017.tga",
    "Sprites/Infobar/018.tga",
    "Sprites/Infobar/019.tga",
    "Sprites/Infobar/020.tga",
    "Sprites/Level10_Park/000.tga",
    "Sprites/Level10_Park/001.tga",
    "Sprites/Level10_Park/002.tga",
    "Sprites/Level10_Park/003.tga",
    "Sprites/Level10_Park/004.tga",
    "Sprites/Level10_Park/005.tga",
    "Sprites/Level10_Park/006.tga",
    "Sprites/Level10_Park/007.tga",
    "Sprites/Level10_Park/008.tga",
    "Sprites/Level10_Park/009.tga",
    "Sprites/Level1_Garden/000.tga",
    "Sprites/Level1_Garden/001.tga",
    "Sprites/Level1_Garden/002.tga",
    "Sprites/Level1_Garden/003.tga",
    "Sprites/Level1_Garden/004.tga",
    "Sprites/Level2_Sidewalk/000.tga",
    "Sprites/Level2_Sidewalk/001.tga",
    "Sprites/Level3_DogHair/000.tga",
    "Sprites/Level3_DogHair/001.tga",
    "Sprites/Level3_DogHair/002.tga",
    "Sprites/Level3_DogHair/003.tga",
    "Sprites/Level3_DogHair/004.tga",
    "Sprites/Level4_Plumbing/000.tga",
    "Sprites/Level4_Plumbing/001.tga",
    "Sprites/Level4_Plumbing/002.tga",
    "Sprites/Level4_Plumbing/003.tga",
    "Sprites/Level4_Plumbing/004.tga",
    "Sprites/Level4_Plumbing/005.tga",
    "Sprites/Level5_Playroom/000.tga",
    "Sprites/Level5_Playroom/001.tga",
    "Sprites/Level5_Playroom/002.tga",
    "Sprites/Level5_Playroom/003.tga",
    "Sprites/Level6_Closet/000.tga",
    "Sprites/Level6_Closet/001.tga",
    "Sprites/Level6_Closet/002.tga",
    "Sprites/Level6_Closet/003.tga",
    "Sprites/Level6_Closet/004.tga",
    "Sprites/Level6_Closet/005.tga",
    "Sprites/Level6_Closet/006.tga",
    "Sprites/Level6_Closet/007.tga",
    "Sprites/Level7_Gutter/000.tga",
    "Sprites/Level8_Garbage/000.tga",
    "Sprites/Level8_Garbage/001.tga",
    "Sprites/Level8_Garbage/002.tga",
    "Sprites/Level8_Garbage/003.tga",
    "Sprites/Level8_Garbage/004.tga",
    "Sprites/Level9_Balsa/000.tga",
    "Sprites/Level9_Balsa/001.tga",
    "Sprites/Level9_Balsa/002.tga",
    "Sprites/LoseScreen/000.tga",
    "Sprites/MainMenu/000.tga",
    "Sprites/MainMenu/001.tga",
    "Sprites/MainMenu/002.tga",
    "Sprites/MainMenu/003.tga",
    "Sprites/MainMenu/004.tga",
    "Sprites/MainMenu/005.tga",
    "Sprites/MainMenu/006.tga",
    "Sprites/MainMenu/007.tga",
    "Sprites/Pangea/000.tga",
    "Sprites/Particle/000.tga",
    "Sprites/Particle/001.tga",
    "Sprites/Particle/002.tga",
    "Sprites/Particle/003.tga",
    "Sprites/Particle/004.tga",
    "Sprites/Particle/005.tga",
    "Sprites/Particle/006.tga",
    "Sprites/Particle/007.tga",
    "Sprites/Particle/008.tga",
    "Sprites/Particle/009.tga",
    "Sprites/Particle/010.tga",
    "Sprites/Particle/011.tga",
    "Sprites/Particle/012.tga",
    "Sprites/Particle/013.tga",
    "Sprites/Particle/014.tga",
    "Sprites/Particle/015.tga",
    "Sprites/Particle/016.tga",
    "Sprites/Particle/017.tga",
    "Sprites/Particle/018.tga",
    "Sprites/Particle/019.tga",
    "Sprites/Particle/020.tga",
    "Sprites/Particle/021.tga",
    "Sprites/Particle/022.tga",
    "Sprites/Particle/023.tga",
    "Sprites/Particle/024.tga",
    "Sprites/Particle/025.tga",
    "Sprites/Particle/026.tga",
    "Sprites/SphereMap/000.tga",
    "Sprites/SphereMap/001.tga",
    "Sprites/SphereMap/002.tga",
    "Sprites/SphereMap/003.tga",
    "Sprites/SphereMap/004.tga",
    "Sprites/SphereMap/005.tga",
    "Sprites/SphereMap/006.tga",
    "Sprites/SphereMap/007.tga",
    "Sprites/SphereMap/008.tga",
    "Sprites/Title/000.tga",
    "Sprites/Title/001.tga",
    "Sprites/Title/002.tga",
    "Sprites/Title/003.tga",
    "Sprites/Title/004.tga",
    "Sprites/WinScreen/000.tga",
    "System/gamecontrollerdb.txt",
    "System/strings.csv",
    "Terrain/Level10_Park.ter",
    "Terrain/Level10_Park.ter.rsrc",
    "Terrain/Level1_Garden.ter",
    "Terrain/Level1_Garden.ter.rsrc",
    "Terrain/Level2_SideWalk.ter",
    "Terrain/Level2_SideWalk.ter.rsrc",
    "Terrain/Level3_DogHair.ter",
    "Terrain/Level3_DogHair.ter.rsrc",
    "Terrain/Level5_Playroom.ter",
    "Terrain/Level5_Playroom.ter.rsrc",
    "Terrain/Level6_Closet.ter",
    "Terrain/Level6_Closet.ter.rsrc",
    "Terrain/Level8_Garbage.ter",
    "Terrain/Level8_Garbage.ter.rsrc",
    "Terrain/Level9_Balsa.ter",
    "Terrain/Level9_Balsa.ter.rsrc",
    "Terrain/Title.ter",
    "Terrain/Title.ter.rsrc",
    "Tunnels/Gutter.tun",
    "Tunnels/Plumbing.tun",
    NULL
};

// -------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------

// Create every directory component of a file path.
static void MakeDirsFor(const char *path)
{
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
}

// Copy one file from the APK asset bundle to the filesystem.
// assetPath  – relative to the APK asset root (no leading /)
// destPath   – absolute filesystem destination
static bool ExtractOneFile(const char *assetPath, const char *destPath)
{
    // SDL_IOFromFile with a relative path opens directly from the APK assets
    // on Android (uses AAssetManager internally).
    SDL_IOStream *src = SDL_IOFromFile(assetPath, "rb");
    if (!src)
    {
        LOGE("Cannot open asset %s: %s", assetPath, SDL_GetError());
        return false;
    }

    MakeDirsFor(destPath);

    FILE *dst = fopen(destPath, "wb");
    if (!dst)
    {
        SDL_CloseIO(src);
        LOGE("Cannot create %s: %s", destPath, strerror(errno));
        return false;
    }

    char buf[65536];
    size_t n;
    bool ok = true;
    while ((n = SDL_ReadIO(src, buf, sizeof(buf))) > 0)
    {
        if (fwrite(buf, 1, n, dst) != n)
        {
            LOGE("Write error for %s", destPath);
            ok = false;
            break;
        }
    }

    fclose(dst);
    SDL_CloseIO(src);
    return ok;
}

// -------------------------------------------------------------------------
// Public API
// -------------------------------------------------------------------------

bool Android_ExtractAssets(const char *destDir)
{
    // Check if already extracted with the current version.
    // The version file is only written after a complete successful extraction,
    // so a partial extraction (e.g. after a crash mid-way) will be retried.
    char versionFile[1024];
    snprintf(versionFile, sizeof(versionFile), "%s/%s", destDir, EXTRACT_VERSION_FILE);

    FILE *vf = fopen(versionFile, "r");
    if (vf)
    {
        char ver[64] = "";
        if (fgets(ver, sizeof(ver), vf))
        {
            size_t len = strlen(ver);
            while (len > 0 && (ver[len-1] == '\n' || ver[len-1] == '\r'))
                ver[--len] = '\0';

            if (strcmp(ver, EXTRACT_VERSION) == 0)
            {
                fclose(vf);
                LOGI("Assets already extracted (version %s)", EXTRACT_VERSION);
                return true;
            }
        }
        fclose(vf);
    }

    LOGI("Extracting game assets to %s ...", destDir);
    mkdir(destDir, 0755);

    int totalFiles = 0;
    int failedFiles = 0;

    for (int i = 0; kAllDataFiles[i] != NULL; i++)
    {
        char destPath[1024];
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, kAllDataFiles[i]);

        if (!ExtractOneFile(kAllDataFiles[i], destPath))
        {
            LOGE("Failed to extract %s", kAllDataFiles[i]);
            failedFiles++;
        }

        totalFiles++;
    }

    if (failedFiles > 0)
    {
        LOGE("Asset extraction: %d/%d files failed", failedFiles, totalFiles);
        return false;
    }

    // Write version stamp only after all files succeeded.
    vf = fopen(versionFile, "w");
    if (vf)
    {
        fputs(EXTRACT_VERSION "\n", vf);
        fclose(vf);
    }

    LOGI("Asset extraction complete: %d files extracted", totalFiles);
    return true;
}

#endif // __ANDROID__
