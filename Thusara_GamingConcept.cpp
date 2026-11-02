// FinalAssingment.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace tle;

//Constants
//Genarla Const
const float kPI = 3.14159265f;
const float kHalf = 0.5f;
const float kDegreesToRadians = kPI / 180.0f;

const int kStartingscore = 0;
const int kFontsize = 24;

const int kFirstmarble = 0;
const int kSecondmarble = 1;
const int kThirdmarble = 2;

//Camera
const float kCameraX = 0.0f;
const float kCameraY = 30.0f;
const float kCameraZ = -70.0f;
const float kCamerarotateX = 12.0f;


const float kFloorX = 0.0f;
const float kFloorY = 0.0f;
const float kFloorZ = 0.0f;

const float kSkyX = 0.0f;
const float kSkyY = -850.0f;
const float kSkyZ = 0.0f;

const int kMarbleCount = 4;
const float kMarbleRadius = 2.0f;
const float kMarbleY = 2.0f;
const float kMarbleRollSpeed = 8.0f;
const float kMarbleShotSpeed = 180.0f;

const float kFrontResetZ = 0.0f;
const float kBackWallZ = 200.0f;
const float kBackResetZ = 205.0f;

const float kMarbleSlotX[kMarbleCount] = { 0.0f, -10.0f, -5.0f, 10.0f };//slots
const float kMarbleSlotZ[kMarbleCount] = { 0.0f, -5.0f, -10.0f, -5.0f };


const int kRowCount = 10;
const int kBlocksPerRow = 9;

const float kBlockWidth = 10.0f;
const float kBlockGap = 2.0f;
const float kBlockY = 0.0f;

const float kFirstRowZ = 130.0f;
const float kRowSpacing = 12.0f;
const float kRowMoveSpeed = 6.0f;
const float kDistanceBeforeNewRow = 12.0f;

const float kHiddenBlockY = -20.0f;
const float kHiddenBlockZ = 1000.0f;
const float kLoseLineZ = 5.0f;
const float kSinkSpeed = 28.0f;

const int kNormalBlockHealth = 2;
const int kHardBlockHealth = 3;
const int kHardBlocksPerRow = 2;


const float kArrowX = 0.0f;
const float kArrowY = 3.0f;
const float kArrowZ = -18.0f;
const float kArrowSpinSpeed = 0.5f;

const float kAimSpeed = 180.0f;
const float kMaxAimAngle = 40.0f;

const float kDummyX = 0.0f;
const float kDummyY = kMarbleY;
const float kDummyZ = 0.0f;


const int kBarrierCountPerSide = 10;
const int kWarningBarrierCount = 4;

const float kLeftBarrierX = -60.0f;
const float kRightBarrierX = 60.0f;
const float kBarrierY = 0.0f;
const float kBarrierStartZ = 0.0f;
const float kBarrierSpacingZ = 18.0f;

const float kBarrierWidth = 12.0f;
const float kBarrierDepth = 18.0f;
const float kBarrierHalfWidth = kBarrierWidth * kHalf;
const float kBarrierHalfDepth = kBarrierDepth * kHalf;
const float kBarrierPushDistance = 0.1f;


const int kScoreTextX = 20;
const int kScoreTextY = 20;

const int kTitleTextX = 520;
const int kTitleTextY = 300;

const int kStartTextX = 470;
const int kStartTextY = 340;

const int kWinTextX = 380;
const int kWinTextY = 320;

const int kLoseTextX = 430;
const int kLoseTextY = 320;

const int kRestartTextX = 470;
const int kRestartTextY = 360;

//Enums
enum Marblestate
{
	MarbleAvailable,
	MarbleReady,
	MarbleFiring,
	MarbleCollision
};
enum Blockstate
{
	Normalblock,
	Damagedblock,
	Sinkingblock,
	Deadblock
};
enum Gamestate
{
	Gamestartscreen,
	Gameready,
	Gamefiring,
	Gamecollision,
	Gameover
};

//global varibles
IModel* marbleModels[kMarbleCount];//3D
float marbleVelocityX[kMarbleCount];
float marbleVelocityZ[kMarbleCount];
Marblestate marbleStates[kMarbleCount];

IModel* blockModels[kRowCount][kBlocksPerRow];//2D
int blockHealth[kRowCount][kBlocksPerRow];
bool blockIsHard[kRowCount][kBlocksPerRow];
Blockstate blockStates[kRowCount][kBlocksPerRow];

IModel* leftBarrierModels[kBarrierCountPerSide];
IModel* rightBarrierModels[kBarrierCountPerSide];

IModel* arrowModel = nullptr;
IModel* dummyModel = nullptr;

Gamestate currentGameState = Gamestartscreen;

int nextReadyMarble = 1;
int totalRowsSpawned = 1;
int score = kStartingscore;

bool playerWon = false;
float aimAngle = 0.0f;
float distanceMovedSinceLastRow = 0.0f;

//Functions
float Radians(float angleInDegrees)
{
	return angleInDegrees * kDegreesToRadians;
}

float AbsoluteValue(float value)
{
	return fabs(value);
}

const char* GetDefaultMarbleSkin(int marbleIndex)
{
	if (marbleIndex == kFirstmarble) return "glass.jpg";
	if (marbleIndex == kSecondmarble) return "glass_blue.jpg";
	if (marbleIndex == kThirdmarble) return "glass_green.jpg";
	return "glass_red.jpg";
}

void ResetMarbleMotion(int marbleIndex)
{
    marbleVelocityX[marbleIndex] = 0.0f;
    marbleVelocityZ[marbleIndex] = 0.0f;
}

void ResetMarbleSlot(int marbleIndex, int slotIndex)
{
    marbleModels[marbleIndex]->SetPosition(kMarbleSlotX[slotIndex], kMarbleY, kMarbleSlotZ[slotIndex]);
    marbleModels[marbleIndex]->ResetOrientation();
    ResetMarbleMotion(marbleIndex);
}

void ResetMarbleToHome(int marbleIndex)
{
    ResetMarbleSlot(marbleIndex, marbleIndex);
    marbleStates[marbleIndex] = MarbleAvailable;
}

void ResetMarbleToLaunchPosition(int marbleIndex)
{
    ResetMarbleSlot(marbleIndex, kFirstmarble);
    marbleStates[marbleIndex] = MarbleReady;     
}

void SetAllMarbleSkins(const char* skinName)
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        marbleModels[marbleIndex]->SetSkin(skinName);
    }
}

void StopAllMarbles()
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        marbleVelocityX[marbleIndex] = 0.0f;
        marbleVelocityZ[marbleIndex] = 0.0f;
    }
}

bool SphereBoxCollision(IModel* marbleModel, IModel* blockModel)
{
    float distanceX = marbleModel->GetX() - blockModel->GetX();
    float distanceZ = marbleModel->GetZ() - blockModel->GetZ();
    float blockHalfSize = kBlockWidth * kHalf;

    return AbsoluteValue(distanceX) <= blockHalfSize + kMarbleRadius &&
        AbsoluteValue(distanceZ) <= blockHalfSize + kMarbleRadius;
}

bool SphereBarrierCollision(IModel* marbleModel, IModel* barrierModel)
{
    float distanceX = marbleModel->GetX() - barrierModel->GetX();
    float distanceZ = marbleModel->GetZ() - barrierModel->GetZ();

    return AbsoluteValue(distanceX) <= kBarrierHalfWidth + kMarbleRadius &&
        AbsoluteValue(distanceZ) <= kBarrierHalfDepth + kMarbleRadius;
}

void ResetBlockData(int rowIndex, int blockIndex)
{
    blockStates[rowIndex][blockIndex] = Deadblock;
    blockHealth[rowIndex][blockIndex] = 0;
    blockIsHard[rowIndex][blockIndex] = false;
}

void SetBlockNormal(int rowIndex, int blockIndex)
{
    blockStates[rowIndex][blockIndex] = Normalblock;
    blockHealth[rowIndex][blockIndex] = kNormalBlockHealth;
    blockIsHard[rowIndex][blockIndex] = false;
    blockModels[rowIndex][blockIndex]->SetSkin("tiles.jpg");
}

void SetBlockHard(int rowIndex, int blockIndex)
{
    blockStates[rowIndex][blockIndex] = Normalblock;
    blockHealth[rowIndex][blockIndex] = kHardBlockHealth;
    blockIsHard[rowIndex][blockIndex] = true;
    blockModels[rowIndex][blockIndex]->SetSkin("tiles_half.jpg");
}

bool IsAnyMarbleReady()
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        if (marbleStates[marbleIndex] == MarbleReady)
        {
            return true;
        }
    }
    return false;
}

void PromoteNextAvailableMarble()
{
    if (currentGameState == Gameover || currentGameState == Gamestartscreen || IsAnyMarbleReady())
    {
        return;
    }

    for (int searchCount = 0; searchCount < kMarbleCount; ++searchCount)
    {
        int marbleIndex = nextReadyMarble + searchCount;

        if (marbleIndex >= kMarbleCount)
        {
            marbleIndex -= kMarbleCount;
        }

        if (marbleStates[marbleIndex] == MarbleAvailable)
        {
            ResetMarbleToLaunchPosition(marbleIndex);
            nextReadyMarble = marbleIndex + 1;

            if (nextReadyMarble >= kMarbleCount)
            {
                nextReadyMarble = 0;
            }
            return;
        }
    }
}

void ResetOneMarble(int marbleIndex)
{
    ResetMarbleToHome(marbleIndex);
    nextReadyMarble = marbleIndex + 1;

    if (nextReadyMarble >= kMarbleCount)
    {
        nextReadyMarble = 0;
    }

    PromoteNextAvailableMarble();
}

void DamageBlock(int rowIndex, int blockIndex)
{
    blockHealth[rowIndex][blockIndex]--;

    if (blockHealth[rowIndex][blockIndex] <= 0)
    {
        blockStates[rowIndex][blockIndex] = Sinkingblock;
    }
    else
    {
        blockStates[rowIndex][blockIndex] = Damagedblock;
        blockModels[rowIndex][blockIndex]->SetSkin("tiles_red.jpg");
    }
}

bool AreAllBlocksDead()
{
    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockStates[rowIndex][blockIndex] != Deadblock)
            {
                return false;
            }
        }
    }
    return true;
}

bool HasAnyBlockReachedLoseLine()
{
    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockStates[rowIndex][blockIndex] != Deadblock &&
                blockModels[rowIndex][blockIndex]->GetZ() <= kLoseLineZ)
            {
                return true;
            }
        }
    }
    return false;
}

float GetFarthestLiveRowZ()
{
    float farthestZ = kFirstRowZ;

    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockStates[rowIndex][blockIndex] != Deadblock &&
                blockModels[rowIndex][blockIndex]->GetZ() > farthestZ)
            {
                farthestZ = blockModels[rowIndex][blockIndex]->GetZ();
            }
        }
    }

    return farthestZ;
}

//Blocks Rows
void SpawnRow(IMesh* blockMesh, int rowIndex, float rowZ)
{
    float totalRowWidth = kBlocksPerRow * kBlockWidth + (kBlocksPerRow - 1) * kBlockGap;
    float firstBlockX = -totalRowWidth * kHalf + kBlockWidth * kHalf;

    for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
    {
        float blockX = firstBlockX + blockIndex * (kBlockWidth + kBlockGap);

        if (blockModels[rowIndex][blockIndex] == nullptr)
        {
            blockModels[rowIndex][blockIndex] = blockMesh->CreateModel(blockX, kBlockY, rowZ);
        }
        else
        {
            blockModels[rowIndex][blockIndex]->SetPosition(blockX, kBlockY, rowZ);
        }

        SetBlockNormal(rowIndex, blockIndex);
    }

    int hardBlocksPlaced = 0;

    while (hardBlocksPlaced < kHardBlocksPerRow)
    {
        int randomBlockIndex = rand() % kBlocksPerRow;

        if (!blockIsHard[rowIndex][randomBlockIndex])
        {
            SetBlockHard(rowIndex, randomBlockIndex);
            hardBlocksPlaced++;
        }
    }
}

void AddNewRowIfPossible(IMesh* blockMesh)
{
    if (totalRowsSpawned >= kRowCount)
    {
        return;
    }

    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        bool rowIsEmpty = true;

        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockStates[rowIndex][blockIndex] != Deadblock)
            {
                rowIsEmpty = false;
            }
        }

        if (rowIsEmpty)
        {
            SpawnRow(blockMesh, rowIndex, GetFarthestLiveRowZ() + kRowSpacing);
            totalRowsSpawned++;
            return;
        }
    }
}

void ClearAllRows()
{
    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockModels[rowIndex][blockIndex] != nullptr)
            {
                blockModels[rowIndex][blockIndex]->SetPosition(0.0f, kHiddenBlockY, kHiddenBlockZ);
            }

            ResetBlockData(rowIndex, blockIndex);
        }
    }
}

void UpdateSinkingBlocks(float frameTime)
{
    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            if (blockStates[rowIndex][blockIndex] == Sinkingblock)
            {
                blockModels[rowIndex][blockIndex]->MoveY(-kSinkSpeed * frameTime);

                if (blockModels[rowIndex][blockIndex]->GetY() <= kHiddenBlockY)
                {
                    blockStates[rowIndex][blockIndex] = Deadblock;
                    blockModels[rowIndex][blockIndex]->SetPosition(
                        blockModels[rowIndex][blockIndex]->GetX(),
                        kHiddenBlockY,
                        kHiddenBlockZ
                    );
                    score++;
                }
            }
        }
    }
}

void UpdateContactState()
{
    bool marbleStillMoving = false;

    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        if (marbleStates[marbleIndex] == MarbleCollision)
        {
            marbleStates[marbleIndex] = MarbleFiring;
        }

        if (marbleStates[marbleIndex] == MarbleFiring)
        {
            marbleStillMoving = true;
        }
    }

    if (marbleStillMoving)
    {
        currentGameState = Gamefiring;
    }
    else
    {
        currentGameState = Gameready;
    }
}

void FireReadyMarble()
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        if (marbleStates[marbleIndex] == MarbleReady)
        {
            float angleInRadians = Radians(aimAngle);

            marbleVelocityX[marbleIndex] = sin(angleInRadians) * kMarbleShotSpeed;
            marbleVelocityZ[marbleIndex] = cos(angleInRadians) * kMarbleShotSpeed;

            marbleStates[marbleIndex] = MarbleFiring;
            currentGameState = Gamefiring;

            PromoteNextAvailableMarble();
            return;
        }
    }
}

void UpdateFiringMarbles(float frameTime)
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        if (marbleStates[marbleIndex] != MarbleFiring)
        {
            continue;
        }

        float movementX = marbleVelocityX[marbleIndex] * frameTime;
        float movementZ = marbleVelocityZ[marbleIndex] * frameTime;

        marbleModels[marbleIndex]->MoveX(movementX);
        marbleModels[marbleIndex]->MoveZ(movementZ);
        marbleModels[marbleIndex]->RotateX(movementZ * kMarbleRollSpeed);
        marbleModels[marbleIndex]->RotateZ(-movementX * kMarbleRollSpeed);

        if (marbleModels[marbleIndex]->GetZ() >= kBackWallZ)
        {
            marbleVelocityZ[marbleIndex] = -marbleVelocityZ[marbleIndex];
            marbleModels[marbleIndex]->SetPosition(
                marbleModels[marbleIndex]->GetX(),
                kMarbleY,
                kBackWallZ - kMarbleRadius
            );
        }

        for (int barrierIndex = 0; barrierIndex < kBarrierCountPerSide; ++barrierIndex)
        {
            if (SphereBarrierCollision(marbleModels[marbleIndex], leftBarrierModels[barrierIndex]))
            {
                marbleVelocityX[marbleIndex] = -marbleVelocityX[marbleIndex];
                marbleModels[marbleIndex]->MoveX(kBarrierPushDistance);
            }

            if (SphereBarrierCollision(marbleModels[marbleIndex], rightBarrierModels[barrierIndex]))
            {
                marbleVelocityX[marbleIndex] = -marbleVelocityX[marbleIndex];
                marbleModels[marbleIndex]->MoveX(-kBarrierPushDistance);
            }
        }

        for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
        {
            for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
            {
                if (blockStates[rowIndex][blockIndex] == Deadblock ||
                    blockStates[rowIndex][blockIndex] == Sinkingblock)
                {
                    continue;
                }

                if (SphereBoxCollision(marbleModels[marbleIndex], blockModels[rowIndex][blockIndex]))
                {
                    float distanceX = marbleModels[marbleIndex]->GetX() - blockModels[rowIndex][blockIndex]->GetX();
                    float distanceZ = marbleModels[marbleIndex]->GetZ() - blockModels[rowIndex][blockIndex]->GetZ();

                    DamageBlock(rowIndex, blockIndex);

                    if (AbsoluteValue(distanceX) > AbsoluteValue(distanceZ))
                    {
                        marbleVelocityX[marbleIndex] = -marbleVelocityX[marbleIndex];
                    }
                    else
                    {
                        marbleVelocityZ[marbleIndex] = -marbleVelocityZ[marbleIndex];
                    }

                    marbleStates[marbleIndex] = MarbleCollision;
                    currentGameState = Gamecollision;
                }
            }
        }

        if (marbleModels[marbleIndex]->GetZ() <= kFrontResetZ ||
            marbleModels[marbleIndex]->GetZ() > kBackResetZ)
        {
            ResetOneMarble(marbleIndex);
        }
    }
}

void ResetAllMarbles()
{
    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        ResetMarbleSlot(marbleIndex, marbleIndex);
        marbleModels[marbleIndex]->SetSkin(GetDefaultMarbleSkin(marbleIndex));
        marbleStates[marbleIndex] = MarbleAvailable;
    }

    marbleStates[kFirstmarble] = MarbleReady;
    nextReadyMarble = 1;
}

void FullReset(IMesh* blockMesh)
{
    aimAngle = 0.0f;
    distanceMovedSinceLastRow = 0.0f;
    totalRowsSpawned = 1;
    score = kStartingscore;
    playerWon = false;

    ResetAllMarbles();

    dummyModel->ResetOrientation();
    arrowModel->AttachToParent(dummyModel);
    arrowModel->SetPosition(kArrowX, kArrowY, kArrowZ);
    arrowModel->ResetOrientation();

    ClearAllRows();
    SpawnRow(blockMesh, 0, kFirstRowZ);

    currentGameState = Gamestartscreen;
}

void main()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "C:\\Users\\thusa\\OneDrive\\Desktop\\UNI\\Gaming Consepts\\Assingment\\Assingment_GamingConsept\\Media" );

	/**** Set up your scene here ****/
	ICamera* camera = myEngine->CreateCamera(kManual, kCameraX, kCameraY, kCameraZ);
	camera->RotateX(kCamerarotateX);

	IMesh* floorMesh = myEngine->LoadMesh("Floor.x");
	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox_Hell.x");
	IMesh* blockMesh = myEngine->LoadMesh("Block.x");
	IMesh* marbleMesh = myEngine->LoadMesh("Marble.x");
	IMesh* arrowMesh = myEngine->LoadMesh("Arrow.x");
	IMesh* barrierMesh = myEngine->LoadMesh("Barrier.x");
	IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");

	IFont* gameFont = myEngine->LoadFont("Arial", kFontsize);

	floorMesh->CreateModel(kFloorX, kFloorY, kFloorZ);
	skyboxMesh->CreateModel(kSkyX, kSkyY, kSkyZ);

    for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
    {
        for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
        {
            blockModels[rowIndex][blockIndex] = nullptr;
            ResetBlockData(rowIndex, blockIndex);
        }
    }

    for (int marbleIndex = 0; marbleIndex < kMarbleCount; ++marbleIndex)
    {
        marbleModels[marbleIndex] = marbleMesh->CreateModel(
            kMarbleSlotX[marbleIndex],
            kMarbleY,
            kMarbleSlotZ[marbleIndex]
        );

        marbleModels[marbleIndex]->SetSkin(GetDefaultMarbleSkin(marbleIndex));
        marbleStates[marbleIndex] = MarbleAvailable;
        ResetMarbleMotion(marbleIndex);
    }

    marbleStates[kFirstmarble] = MarbleReady;

    dummyModel = dummyMesh->CreateModel(kDummyX, kDummyY, kDummyZ);
    arrowModel = arrowMesh->CreateModel(kArrowX, kArrowY, kArrowZ);
    arrowModel->AttachToParent(dummyModel);
    arrowModel->SetSkin("glass_blue.jpg");

    for (int barrierIndex = 0; barrierIndex < kBarrierCountPerSide; ++barrierIndex)
    {
        float barrierZ = kBarrierStartZ + barrierIndex * kBarrierSpacingZ;

        leftBarrierModels[barrierIndex] = barrierMesh->CreateModel(kLeftBarrierX, kBarrierY, barrierZ);
        rightBarrierModels[barrierIndex] = barrierMesh->CreateModel(kRightBarrierX, kBarrierY, barrierZ);

        if (barrierIndex >= kBarrierCountPerSide - kWarningBarrierCount)
        {
            leftBarrierModels[barrierIndex]->SetSkin("barrier1a.bmp");
            rightBarrierModels[barrierIndex]->SetSkin("barrier1a.bmp");
        }
        else
        {
            leftBarrierModels[barrierIndex]->SetSkin("barrier1.bmp");
            rightBarrierModels[barrierIndex]->SetSkin("barrier1.bmp");
        }
    }
    SpawnRow(blockMesh, 0, kFirstRowZ); //

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

        if (myEngine->KeyHit(Key_Escape)) {
            break;
        }

        float frametime = myEngine->Timer();
        if (currentGameState == Gamestartscreen)
        {
            gameFont->Draw("Killer Quebes", kTitleTextX, kTitleTextY);
            gameFont->Draw("Press ENTER to Start", kStartTextX, kStartTextY);

            if (myEngine->KeyHit(Key_Return))
            {
                currentGameState = Gameready;
            }

            continue;
        }

        gameFont->Draw("Score: " + std::to_string(score), kScoreTextX, kScoreTextY);

        if (currentGameState != Gameover)
        {
            if (myEngine->KeyHeld(Key_Z) && aimAngle > -kMaxAimAngle)
            {
                aimAngle -= kAimSpeed * frametime;
            }

            if (myEngine->KeyHeld(Key_X) && aimAngle < kMaxAimAngle)
            {
                aimAngle += kAimSpeed * frametime;
            }
        }

        dummyModel->ResetOrientation();
        dummyModel->RotateY(aimAngle);
        arrowModel->RotateZ(kArrowSpinSpeed);

        if (currentGameState != Gameover)
        {
            for (int rowIndex = 0; rowIndex < kRowCount; ++rowIndex)
            {
                for (int blockIndex = 0; blockIndex < kBlocksPerRow; ++blockIndex)
                {
                    if (blockStates[rowIndex][blockIndex] != Deadblock)
                    {
                        blockModels[rowIndex][blockIndex]->MoveZ(-kRowMoveSpeed * frametime);
                    }
                }
            }

            distanceMovedSinceLastRow += kRowMoveSpeed * frametime;

            if (distanceMovedSinceLastRow >= kDistanceBeforeNewRow)
            {
                AddNewRowIfPossible(blockMesh);
                distanceMovedSinceLastRow -= kDistanceBeforeNewRow;
            }
        }

        UpdateSinkingBlocks(frametime);

        if (HasAnyBlockReachedLoseLine())
        {
            currentGameState = Gameover;
            playerWon = false;
            SetAllMarbleSkins("glass_red.jpg");
        }

        if (currentGameState != Gameover && myEngine->KeyHit(Key_Space))
        {
            FireReadyMarble();
        }

        UpdateFiringMarbles(frametime);

        if (currentGameState == Gamecollision)
        {
            UpdateContactState();
        }

        if (currentGameState != Gameover && AreAllBlocksDead())
        {
            currentGameState = Gameover;
            playerWon = true;
            SetAllMarbleSkins("glass_green.jpg");
        }

        if (currentGameState == Gameover)
        {
            StopAllMarbles();

            if (playerWon){
                gameFont->Draw("All Boxes Destroyed - You Win!", kWinTextX, kWinTextY);
            }
            else{
                gameFont->Draw("The Quebes Killer is Dead!", kLoseTextX, kLoseTextY);
            }

            gameFont->Draw("Press R to Restart", kRestartTextX, kRestartTextY);

            if (myEngine->KeyHit(Key_R)){
                FullReset(blockMesh);
            }
        }
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}