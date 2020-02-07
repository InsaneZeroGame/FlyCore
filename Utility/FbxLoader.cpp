#include "FbxLoader.h"
#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

using namespace fbxsdk;

static void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
static void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
static void DisplayMetaDataConnections(FbxObject* pNode);
static void DisplayString(const char* pHeader, const char* pValue = "", const char* pSuffix = "");
static void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
static void DisplayInt(const char* pHeader, int pValue, const char* pSuffix = "");
static void DisplayDouble(const char* pHeader, double pValue, const char* pSuffix = "");
static void Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix = "");
static void Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
static void DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix = "");
static void Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
static void DisplayTexture(FbxGeometry* pGeometry);
static void DisplayLink(FbxGeometry* pGeometry);
static void DisplayShape(FbxGeometry* pGeometry);
static void DisplayCache(FbxGeometry* pGeometry);
static void DisplayMaterial(FbxGeometry* pGeometry);
static const unsigned int MAT_HEADER_LENGTH = 200;


Utility::FbxLoader::FbxLoader():
    m_sdkManager(FbxManager::Create()),
    m_cameraArray(),
    m_poseArray(),
    m_supportVBO(true)
{

}



void Utility::FbxLoader::LoadFile(const std::string p_fileName, Interface::IScene* p_scene)
{
    FbxScene* m_scene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(m_sdkManager, m_scene);
    // Load the scene.
    
    auto lResult = LoadScene(m_sdkManager, m_scene, p_fileName.c_str());

    if (lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");
    }
    else
    {
        // Display the scene.
        //DisplayMetaData(m_scene);
        //
        //FBXSDK_printf("\n\n---------------------\nGlobal Light Settings\n---------------------\n\n");
        //
        //if (gVerbose) DisplayGlobalLightSettings(&m_scene->GetGlobalSettings());
        //
        //FBXSDK_printf("\n\n----------------------\nGlobal Camera Settings\n----------------------\n\n");
        //
        //if (gVerbose) DisplayGlobalCameraSettings(&m_scene->GetGlobalSettings());
        //
        //FBXSDK_printf("\n\n--------------------\nGlobal Time Settings\n--------------------\n\n");
        //
        //if (gVerbose) DisplayGlobalTimeSettings(&m_scene->GetGlobalSettings());
        //
        //FBXSDK_printf("\n\n---------\nHierarchy\n---------\n\n");
        //
        //if (gVerbose) DisplayHierarchy(m_scene);
        //
        //FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");
        //
        DisplayContent(m_scene);
        
        //FBXSDK_printf("\n\n----\nPose\n----\n\n");
        //
        //if (gVerbose) DisplayPose(m_scene);
        //
        //FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");
        //
        //if (gVerbose) DisplayAnimation(m_scene);
        //
        ////now display generic information
        //
        //FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
        //if (gVerbose) DisplayGenericInfo(m_scene);
    }
}


Utility::FbxLoader::~FbxLoader()
{
    DestroySdkObjects(m_sdkManager, true);

}


bool Utility::FbxLoader::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor, lSDKMinor, lSDKRevision;
    //int lFileFormat = -1;
    int lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pManager, "");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if (!lImportStatus)
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for (int i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);

    // Convert mesh, NURBS and patch into triangle mesh
    FbxGeometryConverter lGeomConverter(m_sdkManager);
    try {
        lGeomConverter.Triangulate(FbxCast<FbxScene>(pScene), /*replace*/true);
    }
    catch (std::runtime_error) {
        FBXSDK_printf("Scene integrity verification failed.\n");
        return false;
    }
    if (lStatus == true)
    {
        // Check the scene integrity!
        FbxStatus status;
        FbxArray< FbxString*> details;
        FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(pScene), &status, &details);
        lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
        bool lNotify = (!lStatus && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
        if (lNotify)
        {
            FBXSDK_printf("\n");
            FBXSDK_printf("********************************************************************************\n");
            if (details.GetCount())
            {
                FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
                for (int i = 0; i < details.GetCount(); i++)
                    FBXSDK_printf("   %s\n", details[i]->Buffer());

                FbxArrayDelete<FbxString*>(details);
            }

            if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
            {
                FBXSDK_printf("\n");
                FBXSDK_printf("WARNING:\n");
                FBXSDK_printf("   The importer was able to read the file but with errors.\n");
                FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
                FBXSDK_printf("   Last error message:'%s'\n", lImporter->GetStatus().GetErrorString());
            }
            FBXSDK_printf("********************************************************************************\n");
            FBXSDK_printf("\n");
        }
    }

    if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
    {
        FBXSDK_printf("Please enter password: ");

        lPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(pScene);

        if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

void Utility::FbxLoader::DisplayContent(FbxScene* pScene)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if (lNode)
    {
        for (i = 0; i < lNode->GetChildCount(); i++)
        {
            DisplayContent(lNode->GetChild(i));
        }
    }
}

void Utility::FbxLoader::DisplayContent(FbxNode* pNode)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if (pNode->GetNodeAttribute() == NULL)
    {
        FBXSDK_printf("NULL Node Attribute\n\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        default:
            break;
        case FbxNodeAttribute::eMarker:
            //DisplayMarker(pNode);
            break;

        case FbxNodeAttribute::eSkeleton:
            //DisplaySkeleton(pNode);
            break;

        case FbxNodeAttribute::eMesh:
            DisplayMesh(pNode);
            break;

        case FbxNodeAttribute::eNurbs:
            //DisplayNurb(pNode);
            break;

        case FbxNodeAttribute::ePatch:
            //DisplayPatch(pNode);
            break;

        case FbxNodeAttribute::eCamera:
            //DisplayCamera(pNode);
            break;

        case FbxNodeAttribute::eLight:
            //DisplayLight(pNode);
            break;

        case FbxNodeAttribute::eLODGroup:
            //DisplayLodGroup(pNode);
            break;
        }
    }

    //DisplayUserProperties(pNode);
    //DisplayTarget(pNode);
    //DisplayPivotsAndLimits(pNode);
    //DisplayTransformPropagation(pNode);
    //DisplayGeometricTransform(pNode);

    for (i = 0; i < pNode->GetChildCount(); i++)
    {
        DisplayContent(pNode->GetChild(i));
    }
}


void Utility::FbxLoader::DisplayMesh(FbxNode* pNode)
{
    FbxMesh* lMesh = (FbxMesh*)pNode->GetNodeAttribute();

    DisplayString("Mesh Name: ", (char*)pNode->GetName());
    DisplayMetaDataConnections(lMesh);
    DisplayControlsPoints(lMesh);
    DisplayPolygons(lMesh);
    DisplayMaterialMapping(lMesh);
    DisplayMaterial(lMesh);
    DisplayTexture(lMesh);
    DisplayMaterialConnections(lMesh);
    DisplayLink(lMesh);
    DisplayShape(lMesh);

    DisplayCache(lMesh);
}

void DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode)
{
    FbxFileTexture* lFileTexture = FbxCast<FbxFileTexture>(pTexture);
    FbxProceduralTexture* lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

    DisplayString("            Name: \"", (char*)pTexture->GetName(), "\"");
    if (lFileTexture)
    {
        DisplayString("            Type: File Texture");
        DisplayString("            File Name: \"", (char*)lFileTexture->GetFileName(), "\"");
    }
    else if (lProceduralTexture)
    {
        DisplayString("            Type: Procedural Texture");
    }
    DisplayDouble("            Scale U: ", pTexture->GetScaleU());
    DisplayDouble("            Scale V: ", pTexture->GetScaleV());
    DisplayDouble("            Translation U: ", pTexture->GetTranslationU());
    DisplayDouble("            Translation V: ", pTexture->GetTranslationV());
    DisplayBool("            Swap UV: ", pTexture->GetSwapUV());
    DisplayDouble("            Rotation U: ", pTexture->GetRotationU());
    DisplayDouble("            Rotation V: ", pTexture->GetRotationV());
    DisplayDouble("            Rotation W: ", pTexture->GetRotationW());

    const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };

    DisplayString("            Alpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
    DisplayDouble("            Cropping Left: ", pTexture->GetCroppingLeft());
    DisplayDouble("            Cropping Top: ", pTexture->GetCroppingTop());
    DisplayDouble("            Cropping Right: ", pTexture->GetCroppingRight());
    DisplayDouble("            Cropping Bottom: ", pTexture->GetCroppingBottom());

    const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical",
        "Box", "Face", "UV", "Environment" };

    DisplayString("            Mapping Type: ", lMappingTypes[pTexture->GetMappingType()]);

    if (pTexture->GetMappingType() == FbxTexture::ePlanar)
    {
        const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };

        DisplayString("            Planar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);
    }

    const char* lBlendModes[] = { "Translucent", "Additive", "Modulate", "Modulate2", "Over", "Normal", "Dissolve", "Darken", "ColorBurn", "LinearBurn",
                                    "DarkerColor", "Lighten", "Screen", "ColorDodge", "LinearDodge", "LighterColor", "SoftLight", "HardLight", "VividLight",
                                    "LinearLight", "PinLight", "HardMix", "Difference", "Exclusion", "Substract", "Divide", "Hue", "Saturation", "Color",
                                    "Luminosity", "Overlay" };

    if (pBlendMode >= 0)
        DisplayString("            Blend Mode: ", lBlendModes[pBlendMode]);
    DisplayDouble("            Alpha: ", pTexture->GetDefaultAlpha());

    if (lFileTexture)
    {
        const char* lMaterialUses[] = { "Model Material", "Default Material" };
        DisplayString("            Material Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);
    }

    const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map",
        "Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };

    DisplayString("            Texture Use: ", pTextureUses[pTexture->GetTextureUse()]);
    DisplayString("");

}

void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex) {

    if (pProperty.IsValid())
    {
        int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

        for (int j = 0; j < lTextureCount; ++j)
        {
            //Here we have to check if it's layeredtextures, or just textures:
            FbxLayeredTexture* lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
            if (lLayeredTexture)
            {
                DisplayInt("    Layered Texture: ", j);
                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
                for (int k = 0; k < lNbTextures; ++k)
                {
                    FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
                    if (lTexture)
                    {

                        if (pDisplayHeader) {
                            DisplayInt("    Textures connected to Material ", pMaterialIndex);
                            pDisplayHeader = false;
                        }

                        //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
                        //Why is that?  because one texture can be shared on different layered textures and might
                        //have different blend modes.

                        FbxLayeredTexture::EBlendMode lBlendMode;
                        lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
                        DisplayString("    Textures for ", pProperty.GetName());
                        DisplayInt("        Texture ", k);
                        DisplayTextureInfo(lTexture, (int)lBlendMode);
                    }

                }
            }
            else
            {
                //no layered texture simply get on the property
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if (lTexture)
                {
                    //display connected Material header only at the first time
                    if (pDisplayHeader) {
                        DisplayInt("    Textures connected to Material ", pMaterialIndex);
                        pDisplayHeader = false;
                    }

                    DisplayString("    Textures for ", pProperty.GetName());
                    DisplayInt("        Texture ", j);
                    DisplayTextureInfo(lTexture, -1);
                }
            }
        }
    }//end if pProperty

}

void DisplayTexture(FbxGeometry* pGeometry)
{
    int lMaterialIndex;
    FbxProperty lProperty;
    if (pGeometry->GetNode() == NULL)
        return;
    int lNbMat = pGeometry->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
    for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++) {
        FbxSurfaceMaterial* lMaterial = pGeometry->GetNode()->GetSrcObject<FbxSurfaceMaterial>(lMaterialIndex);
        bool lDisplayHeader = true;

        //go through all the possible textures
        if (lMaterial) {

            int lTextureIndex;
            FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
            {
                lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
                FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex);
            }

        }//end if(lMaterial)

    }// end for lMaterialIndex     
}

void Utility::FbxLoader::DisplayControlsPoints(FbxMesh* pMesh)
{
    int i, lControlPointsCount = pMesh->GetControlPointsCount();
    FbxVector4* lControlPoints = pMesh->GetControlPoints();

    DisplayString("    Control Points");

    for (i = 0; i < lControlPointsCount; i++)
    {
        DisplayInt("        Control Point ", i);
        Display3DVector("            Coordinates: ", lControlPoints[i]);

        for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
        {
            FbxGeometryElementNormal* leNormals = pMesh->GetElementNormal(j);
            if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
            {
                char header[100];
                FBXSDK_sprintf(header, 100, "            Normal Vector: ");
                if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
                    Display3DVector(header, leNormals->GetDirectArray().GetAt(i));
            }
        }
    }

    DisplayString("");
}


void Utility::FbxLoader::DisplayPolygons(FbxMesh* pMesh)
{
    int i, j, lPolygonCount = pMesh->GetPolygonCount();
    FbxVector4* lControlPoints = pMesh->GetControlPoints();
    char header[100];

    DisplayString("    Polygons");

    int vertexId = 0;
    for (i = 0; i < lPolygonCount; i++)
    {
        DisplayInt("        Polygon ", i);
        int l;

        for (l = 0; l < pMesh->GetElementPolygonGroupCount(); l++)
        {
            FbxGeometryElementPolygonGroup* lePolgrp = pMesh->GetElementPolygonGroup(l);
            switch (lePolgrp->GetMappingMode())
            {
            case FbxGeometryElement::eByPolygon:
                if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
                {
                    FBXSDK_sprintf(header, 100, "        Assigned to group: ");
                    int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
                    DisplayInt(header, polyGroupId);
                    break;
                }
            default:
                // any other mapping modes don't make sense
                DisplayString("        \"unsupported group assignment\"");
                break;
            }
        }

        int lPolygonSize = pMesh->GetPolygonSize(i);

        for (j = 0; j < lPolygonSize; j++)
        {
            int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
            if (lControlPointIndex < 0)
            {
                DisplayString("            Coordinates: Invalid index found!");
                continue;
            }
            else
                Display3DVector("            Coordinates: ", lControlPoints[lControlPointIndex]);

            for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
            {
                FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor(l);
                FBXSDK_sprintf(header, 100, "            Color vertex: ");

                switch (leVtxc->GetMappingMode())
                {
                default:
                    break;
                case FbxGeometryElement::eByControlPoint:
                    switch (leVtxc->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        DisplayColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
                        DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                    break;

                case FbxGeometryElement::eByPolygonVertex:
                {
                    switch (leVtxc->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leVtxc->GetIndexArray().GetAt(vertexId);
                        DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
                break;

                case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                    break;
                }
            }
            for (l = 0; l < pMesh->GetElementUVCount(); ++l)
            {
                FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
                FBXSDK_sprintf(header, 100, "            Texture UV: ");

                switch (leUV->GetMappingMode())
                {
                default:
                    break;
                case FbxGeometryElement::eByControlPoint:
                    switch (leUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        Display2DVector(header, leUV->GetDirectArray().GetAt(lControlPointIndex));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
                        Display2DVector(header, leUV->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                    break;

                case FbxGeometryElement::eByPolygonVertex:
                {
                    int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
                    switch (leUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        Display2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
                break;

                case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                    break;
                }
            }
            for (l = 0; l < pMesh->GetElementNormalCount(); ++l)
            {
                FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
                FBXSDK_sprintf(header, 100, "            Normal: ");

                if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                {
                    switch (leNormal->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        Display3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leNormal->GetIndexArray().GetAt(vertexId);
                        Display3DVector(header, leNormal->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }

            }
            for (l = 0; l < pMesh->GetElementTangentCount(); ++l)
            {
                FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
                FBXSDK_sprintf(header, 100, "            Tangent: ");

                if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                {
                    switch (leTangent->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        Display3DVector(header, leTangent->GetDirectArray().GetAt(vertexId));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leTangent->GetIndexArray().GetAt(vertexId);
                        Display3DVector(header, leTangent->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }

            }
            for (l = 0; l < pMesh->GetElementBinormalCount(); ++l)
            {

                FbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal(l);

                FBXSDK_sprintf(header, 100, "            Binormal: ");
                if (leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                {
                    switch (leBinormal->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        Display3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leBinormal->GetIndexArray().GetAt(vertexId);
                        Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
            }
            vertexId++;
        } // for polygonSize
    } // for polygonCount


    //check visibility for the edges of the mesh
    for (int l = 0; l < pMesh->GetElementVisibilityCount(); ++l)
    {
        FbxGeometryElementVisibility* leVisibility = pMesh->GetElementVisibility(l);
        FBXSDK_sprintf(header, 100, "    Edge Visibility : ");
        DisplayString(header);
        switch (leVisibility->GetMappingMode())
        {
        default:
            break;
            //should be eByEdge
        case FbxGeometryElement::eByEdge:
            //should be eDirect
            for (j = 0; j != pMesh->GetMeshEdgeCount(); ++j)
            {
                DisplayInt("        Edge ", j);
                DisplayBool("              Edge visibility: ", leVisibility->GetDirectArray().GetAt(j));
            }

            break;
        }
    }
    DisplayString("");
}

void Utility::FbxLoader::DisplayTextureNames(FbxProperty& pProperty, FbxString& pConnectionString)
{
    int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
    if (lLayeredTextureCount > 0)
    {
        for (int j = 0; j < lLayeredTextureCount; ++j)
        {
            FbxLayeredTexture* lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
            int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
            pConnectionString += " Texture ";

            for (int k = 0; k < lNbTextures; ++k)
            {
                //lConnectionString += k;
                pConnectionString += "\"";
                pConnectionString += (char*)lLayeredTexture->GetName();
                pConnectionString += "\"";
                pConnectionString += " ";
            }
            pConnectionString += "of ";
            pConnectionString += pProperty.GetName();
            pConnectionString += " on layer ";
            pConnectionString += j;
        }
        pConnectionString += " |";
    }
    else
    {
        //no layered texture simply get on the property
        int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();

        if (lNbTextures > 0)
        {
            pConnectionString += " Texture ";
            pConnectionString += " ";

            for (int j = 0; j < lNbTextures; ++j)
            {
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if (lTexture)
                {
                    pConnectionString += "\"";
                    pConnectionString += (char*)lTexture->GetName();
                    pConnectionString += "\"";
                    pConnectionString += " ";
                }
            }
            pConnectionString += "of ";
            pConnectionString += pProperty.GetName();
            pConnectionString += " |";
        }
    }
}

void Utility::FbxLoader::DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char* header, int pMatId, int l)
{
    if (!pMaterial)
        return;

    FbxString lConnectionString = "            Material %d -- ";
    //Show all the textures

    FbxProperty lProperty;
    //Diffuse Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
    DisplayTextureNames(lProperty, lConnectionString);

    //DiffuseFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Emissive Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
    DisplayTextureNames(lProperty, lConnectionString);

    //EmissiveFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor);
    DisplayTextureNames(lProperty, lConnectionString);


    //Ambient Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
    DisplayTextureNames(lProperty, lConnectionString);

    //AmbientFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Specular Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
    DisplayTextureNames(lProperty, lConnectionString);

    //SpecularFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Shininess Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    DisplayTextureNames(lProperty, lConnectionString);

    //Bump Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sBump);
    DisplayTextureNames(lProperty, lConnectionString);

    //Normal Map Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
    DisplayTextureNames(lProperty, lConnectionString);

    //Transparent Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor);
    DisplayTextureNames(lProperty, lConnectionString);

    //TransparencyFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Reflection Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflection);
    DisplayTextureNames(lProperty, lConnectionString);

    //ReflectionFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Update header with material info
    bool lStringOverflow = (lConnectionString.GetLen() + 10 >= MAT_HEADER_LENGTH); // allow for string length and some padding for "%d"
    if (lStringOverflow)
    {
        // Truncate string!
        lConnectionString = lConnectionString.Left(MAT_HEADER_LENGTH - 10);
        lConnectionString = lConnectionString + "...";
    }
    FBXSDK_sprintf(header, MAT_HEADER_LENGTH, lConnectionString.Buffer(), pMatId, l);
    DisplayString(header);
}

void Utility::FbxLoader::DisplayMaterialConnections(FbxMesh* pMesh)
{
    int i, l, lPolygonCount = pMesh->GetPolygonCount();

    char header[MAT_HEADER_LENGTH];

    DisplayString("    Polygons Material Connections");

    //check whether the material maps with only one mesh
    bool lIsAllSame = true;
    for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
    {

        FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
        if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
        {
            lIsAllSame = false;
            break;
        }
    }

    //For eAllSame mapping type, just out the material and texture mapping info once
    if (lIsAllSame)
    {
        for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
        {

            FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
            if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame)
            {
                FbxSurfaceMaterial* lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(0));
                int lMatId = lMaterialElement->GetIndexArray().GetAt(0);
                if (lMatId >= 0)
                {
                    DisplayInt("        All polygons share the same material in mesh ", l);
                    DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
                }
            }
        }

        //no material
        if (l == 0)
            DisplayString("        no material applied");
    }

    //For eByPolygon mapping type, just out the material and texture mapping info once
    else
    {
        for (i = 0; i < lPolygonCount; i++)
        {
            DisplayInt("        Polygon ", i);

            for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
            {

                FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
                FbxSurfaceMaterial* lMaterial = NULL;
                int lMatId = -1;
                lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
                lMatId = lMaterialElement->GetIndexArray().GetAt(i);

                if (lMatId >= 0)
                {
                    DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
                }
            }
        }
    }
}


void Utility::FbxLoader::DisplayMaterialMapping(FbxMesh* pMesh)
{
    const char* lMappingTypes[] = { "None", "By Control Point", "By Polygon Vertex", "By Polygon", "By Edge", "All Same" };
    const char* lReferenceMode[] = { "Direct", "Index", "Index to Direct" };

    int lMtrlCount = 0;
    FbxNode* lNode = NULL;
    if (pMesh) {
        lNode = pMesh->GetNode();
        if (lNode)
            lMtrlCount = lNode->GetMaterialCount();
    }

    for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
    {
        FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
        if (leMat)
        {
            char header[100];
            FBXSDK_sprintf(header, 100, "    Material Element %d: ", l);
            DisplayString(header);


            DisplayString("           Mapping: ", lMappingTypes[leMat->GetMappingMode()]);
            DisplayString("           ReferenceMode: ", lReferenceMode[leMat->GetReferenceMode()]);

            int lMaterialCount = 0;
            FbxString lString;

            if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect ||
                leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
            {
                lMaterialCount = lMtrlCount;
            }

            if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex ||
                leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
            {
                int i;

                lString = "           Indices: ";

                int lIndexArrayCount = leMat->GetIndexArray().GetCount();
                for (i = 0; i < lIndexArrayCount; i++)
                {
                    lString += leMat->GetIndexArray().GetAt(i);

                    if (i < lIndexArrayCount - 1)
                    {
                        lString += ", ";
                    }
                }

                lString += "\n";

                FBXSDK_printf(lString);
            }
        }
    }

    DisplayString("");
}


void Utility::FbxLoader::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if (!pManager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
    else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
    if (!pScene)
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}


void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if (pManager) pManager->Destroy();
    if (pExitStatus) FBXSDK_printf("Program Success!\n");
}



void DisplayMaterial(FbxGeometry* pGeometry)
{
    int lMaterialCount = 0;
    FbxNode* lNode = NULL;
    if (pGeometry) {
        lNode = pGeometry->GetNode();
        if (lNode)
            lMaterialCount = lNode->GetMaterialCount();
    }

    if (lMaterialCount > 0)
    {
        FbxPropertyT<FbxDouble3> lKFbxDouble3;
        FbxPropertyT<FbxDouble> lKFbxDouble1;
        FbxColor theColor;

        for (int lCount = 0; lCount < lMaterialCount; lCount++)
        {
            DisplayInt("        Material ", lCount);

            FbxSurfaceMaterial* lMaterial = lNode->GetMaterial(lCount);

            DisplayString("            Name: \"", (char*)lMaterial->GetName(), "\"");

            //Get the implementation to see if it's a hardware shader.
            const FbxImplementation* lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_HLSL);
            FbxString lImplemenationType = "HLSL";
            if (!lImplementation)
            {
                lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_CGFX);
                lImplemenationType = "CGFX";
            }
            if (lImplementation)
            {
                //Now we have a hardware shader, let's read it
                FBXSDK_printf("            Hardware Shader Type: %s\n", lImplemenationType.Buffer());
                const FbxBindingTable* lRootTable = lImplementation->GetRootTable();
                FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
                FbxString lTechniqueName = lRootTable->DescTAG.Get();


                const FbxBindingTable* lTable = lImplementation->GetRootTable();
                size_t lEntryNum = lTable->GetEntryCount();

                for (int i = 0; i < (int)lEntryNum; ++i)
                {
                    const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
                    const char* lEntrySrcType = lEntry.GetEntryType(true);
                    FbxProperty lFbxProp;


                    FbxString lTest = lEntry.GetSource();
                    FBXSDK_printf("            Entry: %s\n", lTest.Buffer());


                    if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
                    {
                        lFbxProp = lMaterial->FindPropertyHierarchical(lEntry.GetSource());
                        if (!lFbxProp.IsValid())
                        {
                            lFbxProp = lMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
                        }


                    }
                    else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
                    {
                        lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
                    }
                    if (lFbxProp.IsValid())
                    {
                        if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
                        {
                            //do what you want with the textures
                            for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
                            {
                                FbxFileTexture* lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
                                FBXSDK_printf("           File Texture: %s\n", lTex->GetFileName());
                            }
                            for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
                            {
                                FbxLayeredTexture* lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
                                FBXSDK_printf("        Layered Texture: %s\n", lTex->GetName());
                            }
                            for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
                            {
                                FbxProceduralTexture* lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
                                FBXSDK_printf("     Procedural Texture: %s\n", lTex->GetName());
                            }
                        }
                        else
                        {
                            FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
                            FbxString blah = lFbxType.GetName();
                            if (FbxBoolDT == lFbxType)
                            {
                                DisplayBool("                Bool: ", lFbxProp.Get<FbxBool>());
                            }
                            else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
                            {
                                DisplayInt("                Int: ", lFbxProp.Get<FbxInt>());
                            }
                            else if (FbxFloatDT == lFbxType)
                            {
                                DisplayDouble("                Float: ", lFbxProp.Get<FbxFloat>());

                            }
                            else if (FbxDoubleDT == lFbxType)
                            {
                                DisplayDouble("                Double: ", lFbxProp.Get<FbxDouble>());
                            }
                            else if (FbxStringDT == lFbxType
                                || FbxUrlDT == lFbxType
                                || FbxXRefUrlDT == lFbxType)
                            {
                                DisplayString("                String: ", lFbxProp.Get<FbxString>().Buffer());
                            }
                            else if (FbxDouble2DT == lFbxType)
                            {
                                FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
                                FbxVector2 lVect;
                                lVect[0] = lDouble2[0];
                                lVect[1] = lDouble2[1];

                                Display2DVector("                2D vector: ", lVect);
                            }
                            else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
                            {
                                FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();


                                FbxVector4 lVect;
                                lVect[0] = lDouble3[0];
                                lVect[1] = lDouble3[1];
                                lVect[2] = lDouble3[2];
                                Display3DVector("                3D vector: ", lVect);
                            }

                            else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
                            {
                                FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
                                FbxVector4 lVect;
                                lVect[0] = lDouble4[0];
                                lVect[1] = lDouble4[1];
                                lVect[2] = lDouble4[2];
                                lVect[3] = lDouble4[3];
                                Display4DVector("                4D vector: ", lVect);
                            }
                            else if (FbxDouble4x4DT == lFbxType)
                            {
                                FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
                                for (int j = 0; j < 4; ++j)
                                {

                                    FbxVector4 lVect;
                                    lVect[0] = lDouble44[j][0];
                                    lVect[1] = lDouble44[j][1];
                                    lVect[2] = lDouble44[j][2];
                                    lVect[3] = lDouble44[j][3];
                                    Display4DVector("                4x4D vector: ", lVect);
                                }

                            }
                        }

                    }
                }
            }
            else if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
            {
                // We found a Phong material.  Display its properties.

                // Display the Ambient Color
                lKFbxDouble3 = ((FbxSurfacePhong*)lMaterial)->Ambient;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Ambient: ", theColor);

                // Display the Diffuse Color
                lKFbxDouble3 = ((FbxSurfacePhong*)lMaterial)->Diffuse;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Diffuse: ", theColor);

                // Display the Specular Color (unique to Phong materials)
                lKFbxDouble3 = ((FbxSurfacePhong*)lMaterial)->Specular;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Specular: ", theColor);

                // Display the Emissive Color
                lKFbxDouble3 = ((FbxSurfacePhong*)lMaterial)->Emissive;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Emissive: ", theColor);

                //Opacity is Transparency factor now
                lKFbxDouble1 = ((FbxSurfacePhong*)lMaterial)->TransparencyFactor;
                DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());

                // Display the Shininess
                lKFbxDouble1 = ((FbxSurfacePhong*)lMaterial)->Shininess;
                DisplayDouble("            Shininess: ", lKFbxDouble1.Get());

                // Display the Reflectivity
                lKFbxDouble1 = ((FbxSurfacePhong*)lMaterial)->ReflectionFactor;
                DisplayDouble("            Reflectivity: ", lKFbxDouble1.Get());
            }
            else if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
            {
                // We found a Lambert material. Display its properties.
                // Display the Ambient Color
                lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Ambient;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Ambient: ", theColor);

                // Display the Diffuse Color
                lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Diffuse;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Diffuse: ", theColor);

                // Display the Emissive
                lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Emissive;
                theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                DisplayColor("            Emissive: ", theColor);

                // Display the Opacity
                lKFbxDouble1 = ((FbxSurfaceLambert*)lMaterial)->TransparencyFactor;
                DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
            }
            else
                DisplayString("Unknown type of Material");

            FbxPropertyT<FbxString> lString;
            lString = lMaterial->ShadingModel;
            DisplayString("            Shading Model: ", lString.Get().Buffer());
            DisplayString("");
        }
    }
}


void DisplayMetaDataConnections(FbxObject* pObject)
{
    int nbMetaData = pObject->GetSrcObjectCount<FbxObjectMetaData>();
    if (nbMetaData > 0)
        DisplayString("    MetaData connections ");

    for (int i = 0; i < nbMetaData; i++)
    {
        FbxObjectMetaData* metaData = pObject->GetSrcObject<FbxObjectMetaData>(i);
        DisplayString("        Name: ", (char*)metaData->GetName());
    }
}

void DisplayString(const char* pHeader, const char* pValue /* = "" */, const char* pSuffix /* = "" */)
{
    FbxString lString;

    lString = pHeader;
    lString += pValue;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;

    lString = pHeader;
    lString += pValue ? "true" : "false";
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void DisplayInt(const char* pHeader, int pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;

    lString = pHeader;
    lString += pValue;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void DisplayDouble(const char* pHeader, double pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;
    FbxString lFloatValue = (float)pValue;

    lFloatValue = pValue <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
    lFloatValue = pValue >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();

    lString = pHeader;
    lString += lFloatValue;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix  /* = "" */)
{
    FbxString lString;
    FbxString lFloatValue1 = (float)pValue[0];
    FbxString lFloatValue2 = (float)pValue[1];

    lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
    lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
    lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
    lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();

    lString = pHeader;
    lString += lFloatValue1;
    lString += ", ";
    lString += lFloatValue2;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;
    FbxString lFloatValue1 = (float)pValue[0];
    FbxString lFloatValue2 = (float)pValue[1];
    FbxString lFloatValue3 = (float)pValue[2];

    lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
    lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
    lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
    lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
    lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
    lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();

    lString = pHeader;
    lString += lFloatValue1;
    lString += ", ";
    lString += lFloatValue2;
    lString += ", ";
    lString += lFloatValue3;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}

void Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;
    FbxString lFloatValue1 = (float)pValue[0];
    FbxString lFloatValue2 = (float)pValue[1];
    FbxString lFloatValue3 = (float)pValue[2];
    FbxString lFloatValue4 = (float)pValue[3];

    lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
    lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
    lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
    lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
    lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
    lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
    lFloatValue4 = pValue[3] <= -HUGE_VAL ? "-INFINITY" : lFloatValue4.Buffer();
    lFloatValue4 = pValue[3] >= HUGE_VAL ? "INFINITY" : lFloatValue4.Buffer();

    lString = pHeader;
    lString += lFloatValue1;
    lString += ", ";
    lString += lFloatValue2;
    lString += ", ";
    lString += lFloatValue3;
    lString += ", ";
    lString += lFloatValue4;
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void DisplayColor(const char* pHeader, FbxPropertyT<FbxDouble3> pValue, const char* pSuffix /* = "" */)

{
    FbxString lString;

    lString = pHeader;
    //lString += (float) pValue.mRed;
    //lString += (double)pValue.GetArrayItem(0);
    lString += " (red), ";
    //lString += (float) pValue.mGreen;
    //lString += (double)pValue.GetArrayItem(1);
    lString += " (green), ";
    //lString += (float) pValue.mBlue;
    //lString += (double)pValue.GetArrayItem(2);
    lString += " (blue)";
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}


void DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix /* = "" */)
{
    FbxString lString;

    lString = pHeader;
    lString += (float)pValue.mRed;

    lString += " (red), ";
    lString += (float)pValue.mGreen;

    lString += " (green), ";
    lString += (float)pValue.mBlue;

    lString += " (blue)";
    lString += pSuffix;
    lString += "\n";
    FBXSDK_printf(lString);
}



void DisplayLink(FbxGeometry* pGeometry)
{
    //Display cluster now

    //int i, lLinkCount;
    //FbxCluster* lLink;

    int i, j;
    int lSkinCount = 0;
    int lClusterCount = 0;
    FbxCluster* lCluster;

    lSkinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);



    //lLinkCount = pGeometry->GetLinkCount();
    for (i = 0; i != lSkinCount; ++i)
    {
        lClusterCount = ((FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
        for (j = 0; j != lClusterCount; ++j)
        {
            DisplayInt("    Cluster ", i);

            lCluster = ((FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);
            //lLink = pGeometry->GetLink(i);    

            const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };

            DisplayString("    Mode: ", lClusterModes[lCluster->GetLinkMode()]);

            if (lCluster->GetLink() != NULL)
            {
                DisplayString("        Name: ", (char*)lCluster->GetLink()->GetName());
            }

            FbxString lString1 = "        Link Indices: ";
            FbxString lString2 = "        Weight Values: ";

            int k, lIndexCount = lCluster->GetControlPointIndicesCount();
            int* lIndices = lCluster->GetControlPointIndices();
            double* lWeights = lCluster->GetControlPointWeights();

            for (k = 0; k < lIndexCount; k++)
            {
                lString1 += lIndices[k];
                lString2 += (float)lWeights[k];

                if (k < lIndexCount - 1)
                {
                    lString1 += ", ";
                    lString2 += ", ";
                }
            }

            lString1 += "\n";
            lString2 += "\n";

            FBXSDK_printf(lString1);
            FBXSDK_printf(lString2);

            DisplayString("");

            FbxAMatrix lMatrix;

            lMatrix = lCluster->GetTransformMatrix(lMatrix);
            Display3DVector("        Transform Translation: ", lMatrix.GetT());
            Display3DVector("        Transform Rotation: ", lMatrix.GetR());
            Display3DVector("        Transform Scaling: ", lMatrix.GetS());

            lMatrix = lCluster->GetTransformLinkMatrix(lMatrix);
            Display3DVector("        Transform Link Translation: ", lMatrix.GetT());
            Display3DVector("        Transform Link Rotation: ", lMatrix.GetR());
            Display3DVector("        Transform Link Scaling: ", lMatrix.GetS());

            if (lCluster->GetAssociateModel() != NULL)
            {
                lMatrix = lCluster->GetTransformAssociateModelMatrix(lMatrix);
                DisplayString("        Associate Model: ", (char*)lCluster->GetAssociateModel()->GetName());
                Display3DVector("        Associate Model Translation: ", lMatrix.GetT());
                Display3DVector("        Associate Model Rotation: ", lMatrix.GetR());
                Display3DVector("        Associate Model Scaling: ", lMatrix.GetS());
            }

            DisplayString("");
        }
    }
}

static void DisplayShapeLayerElements(const FbxShape* pShape, const FbxMesh* pMesh);

void DisplayShape(FbxGeometry* pGeometry)
{
    int lBlendShapeCount, lBlendShapeChannelCount, lTargetShapeCount;
    FbxBlendShape* lBlendShape;
    FbxBlendShapeChannel* lBlendShapeChannel;
    FbxShape* lShape;

    lBlendShapeCount = pGeometry->GetDeformerCount(FbxDeformer::eBlendShape);

    for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeCount; ++lBlendShapeIndex)
    {
        lBlendShape = (FbxBlendShape*)pGeometry->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
        DisplayString("    BlendShape ", (char*)lBlendShape->GetName());

        lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
        for (int lBlendShapeChannelIndex = 0; lBlendShapeChannelIndex < lBlendShapeChannelCount; ++lBlendShapeChannelIndex)
        {
            lBlendShapeChannel = lBlendShape->GetBlendShapeChannel(lBlendShapeChannelIndex);
            DisplayString("    BlendShapeChannel ", (char*)lBlendShapeChannel->GetName());
            DisplayDouble("    Default Deform Value: ", lBlendShapeChannel->DeformPercent.Get());

            lTargetShapeCount = lBlendShapeChannel->GetTargetShapeCount();
            for (int lTargetShapeIndex = 0; lTargetShapeIndex < lTargetShapeCount; ++lTargetShapeIndex)
            {
                lShape = lBlendShapeChannel->GetTargetShape(lTargetShapeIndex);
                DisplayString("    TargetShape ", (char*)lShape->GetName());

                if (pGeometry->GetAttributeType() == FbxNodeAttribute::eMesh)
                {
                    DisplayShapeLayerElements(lShape, FbxCast<FbxMesh>(pGeometry));
                }
                else
                {
                    int j, lControlPointsCount = lShape->GetControlPointsCount();
                    FbxVector4* lControlPoints = lShape->GetControlPoints();
                    FbxLayerElementArrayTemplate<FbxVector4>* lNormals = NULL;
                    bool lStatus = lShape->GetNormals(&lNormals);

                    for (j = 0; j < lControlPointsCount; j++)
                    {
                        DisplayInt("        Control Point ", j);
                        Display3DVector("            Coordinates: ", lControlPoints[j]);

                        if (lStatus && lNormals && lNormals->GetCount() == lControlPointsCount)
                        {
                            Display3DVector("            Normal Vector: ", lNormals->GetAt(j));
                        }
                    }
                }

                DisplayString("");
            }
        }
    }
}

void DisplayElementData(const FbxString& pHeader, const FbxVector4& pData, int index = -1)
{
    FbxString desc(pHeader);
    if (index != -1)
    {
        FbxString num = FbxString(" [") + index + "]: ";
        desc.FindAndReplace(":", num.Buffer());
    }
    Display3DVector(desc.Buffer(), pData);
}

void DisplayElementData(const FbxString& pHeader, const FbxVector2& pData, int index = -1)
{
    FbxString desc(pHeader);
    if (index != -1)
    {
        FbxString num = FbxString(" [") + index + "]: ";
        desc.FindAndReplace(":", num.Buffer());
    }
    Display2DVector(desc.Buffer(), pData);
}

void DisplayElementData(const FbxString& pHeader, const FbxColor& pData, int index = -1)
{
    FbxString desc(pHeader);
    if (index != -1)
    {
        FbxString num = FbxString(" [") + index + "]: ";
        desc.FindAndReplace(":", num.Buffer());
    }
    DisplayColor(desc.Buffer(), pData);
}

void FillHeaderBasedOnElementType(FbxLayerElement::EType pComponent, FbxString& pHeader)
{
    switch (pComponent)
    {
    case FbxLayerElement::eNormal:
        pHeader = "        Normal: ";
        break;
    case FbxLayerElement::eBiNormal:
        pHeader = "        BiNormal: ";
        break;
    case FbxLayerElement::eTangent:
        pHeader = "        Tangent: ";
        break;
    case FbxLayerElement::eUV:
        pHeader = "        UV: ";
        break;
    case FbxLayerElement::eVertexColor:
        pHeader = "        Vertex Color: ";
        break;
    default:
        pHeader = "        Unsupported element: ";
        break;
    }
}

template<class T>
void DisplayLayerElement(FbxLayerElement::EType pComponent, const FbxLayer* pShapeLayer, const FbxMesh* pMesh)
{
    const FbxLayerElement* pLayerElement = pShapeLayer->GetLayerElementOfType(pComponent);
    if (pLayerElement)
    {
        FbxLayerElementTemplate<T>* pLayerElementTemplate = ((FbxLayerElementTemplate<T>*) pLayerElement);
        FbxLayerElementArrayTemplate<T>& pLayerElementArray = pLayerElementTemplate->GetDirectArray();
        FbxString header;
        FillHeaderBasedOnElementType(pComponent, header);
        int lPolygonCount = pMesh->GetPolygonCount();
        int lPolynodeIndex = 0;
        for (int i = 0; i < lPolygonCount; ++i)
        {
            int lPolygonSize = pMesh->GetPolygonSize(i);
            for (int j = 0; j < lPolygonSize; ++j, ++lPolynodeIndex)
            {
                int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
                switch (pLayerElementTemplate->GetMappingMode())
                {
                default:
                    break;
                case FbxGeometryElement::eByControlPoint:
                {
                    switch (pLayerElementTemplate->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        DisplayElementData(header, pLayerElementArray.GetAt(lControlPointIndex), lControlPointIndex);
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        FbxLayerElementArrayTemplate<int>& pLayerElementIndexArray = pLayerElementTemplate->GetIndexArray();
                        int id = pLayerElementIndexArray.GetAt(lControlPointIndex);
                        if (id > 0) { DisplayElementData(header, pLayerElementArray.GetAt(id), id); }
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    };
                }
                case FbxGeometryElement::eByPolygonVertex:
                {
                    switch (pLayerElementTemplate->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        DisplayElementData(header, pLayerElementArray.GetAt(lPolynodeIndex), lPolynodeIndex);
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        FbxLayerElementArrayTemplate<int>& pLayerElementIndexArray = pLayerElementTemplate->GetIndexArray();
                        int id = pLayerElementIndexArray.GetAt(lPolynodeIndex);
                        if (id > 0) { DisplayElementData(header, pLayerElementArray.GetAt(id), id); }
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
                break;
                }
            }
        }
    }
}

void DisplayShapeLayerElements(const FbxShape* pShape, const FbxMesh* pMesh)
{
    int lShapeControlPointsCount = pShape->GetControlPointsCount();
    int lMeshControlPointsCount = pMesh->GetControlPointsCount();
    bool lValidLayerElementSource = pShape && pMesh && lShapeControlPointsCount && lMeshControlPointsCount;
    if (lValidLayerElementSource)
    {
        if (lShapeControlPointsCount == lMeshControlPointsCount)
        {
            // Display control points that are different from the mesh
            FbxVector4* lShapeControlPoint = pShape->GetControlPoints();
            FbxVector4* lMeshControlPoint = pMesh->GetControlPoints();
            for (int j = 0; j < lShapeControlPointsCount; j++)
            {
                FbxVector4 delta = lShapeControlPoint[j] - lMeshControlPoint[j];
                if (!FbxEqual(delta, FbxZeroVector4))
                {
                    FbxString header("        Control Point: ");
                    DisplayElementData(header, lShapeControlPoint[j], j);
                }
            }
        }

        int lLayerCount = pShape->GetLayerCount();
        for (int i = 0; i < lLayerCount; ++i)
        {
            const FbxLayer* pLayer = pShape->GetLayer(i);
            DisplayLayerElement<FbxVector4>(FbxLayerElement::eNormal, pLayer, pMesh);
            DisplayLayerElement<FbxColor>(FbxLayerElement::eVertexColor, pLayer, pMesh);
            DisplayLayerElement<FbxVector4>(FbxLayerElement::eTangent, pLayer, pMesh);
            DisplayLayerElement<FbxColor>(FbxLayerElement::eBiNormal, pLayer, pMesh);
            DisplayLayerElement<FbxColor>(FbxLayerElement::eUV, pLayer, pMesh);
        }
    }
}

void DisplayCache(FbxGeometry* pGeometry)
{
    int lVertexCacheDeformerCount = pGeometry->GetDeformerCount(FbxDeformer::eVertexCache);

    for (int i = 0; i < lVertexCacheDeformerCount; ++i)
    {
        FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pGeometry->GetDeformer(i, FbxDeformer::eVertexCache));
        if (!lDeformer) continue;

        FbxCache* lCache = lDeformer->GetCache();
        if (!lCache) continue;

        if (lCache->OpenFileForRead())
        {
            DisplayString("    Vertex Cache");
            int lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
            // skip normal channel
            if (lChannelIndex < 0)
                continue;

            FbxString lChnlName, lChnlInterp;

            FbxCache::EMCDataType lChnlType;
            FbxTime start, stop, rate;
            FbxCache::EMCSamplingType lChnlSampling;
            unsigned int lChnlSampleCount, lDataCount;

            lCache->GetChannelName(lChannelIndex, lChnlName);
            DisplayString("        Channel Name: ", lChnlName.Buffer());
            lCache->GetChannelDataType(lChannelIndex, lChnlType);
            switch (lChnlType)
            {
            case FbxCache::eUnknownData:
                DisplayString("        Channel Type: Unknown Data"); break;
            case FbxCache::eDouble:
                DisplayString("        Channel Type: Double"); break;
            case FbxCache::eDoubleArray:
                DisplayString("        Channel Type: Double Array"); break;
            case FbxCache::eDoubleVectorArray:
                DisplayString("        Channel Type: Double Vector Array"); break;
            case FbxCache::eInt32Array:
                DisplayString("        Channel Type: Int32 Array"); break;
            case FbxCache::eFloatArray:
                DisplayString("        Channel Type: Float Array"); break;
            case FbxCache::eFloatVectorArray:
                DisplayString("        Channel Type: Float Vector Array"); break;
            }
            lCache->GetChannelInterpretation(lChannelIndex, lChnlInterp);
            DisplayString("        Channel Interpretation: ", lChnlInterp.Buffer());
            lCache->GetChannelSamplingType(lChannelIndex, lChnlSampling);
            DisplayInt("        Channel Sampling Type: ", lChnlSampling);
            lCache->GetAnimationRange(lChannelIndex, start, stop);
            lCache->GetChannelSamplingRate(lChannelIndex, rate);
            lCache->GetChannelSampleCount(lChannelIndex, lChnlSampleCount);
            DisplayInt("        Channel Sample Count: ", lChnlSampleCount);

            // Only display cache data if the data type is float vector array
            if (lChnlType != FbxCache::eFloatVectorArray)
                continue;

            if (lChnlInterp == "normals")
                DisplayString("        Normal Cache Data");
            else
                DisplayString("        Points Cache Data");
            float* lBuffer = NULL;
            unsigned int lBufferSize = 0;
            int lFrame = 0;
            for (FbxTime t = start; t <= stop; t += rate)
            {
                DisplayInt("            Frame ", lFrame);
                lCache->GetChannelPointCount(lChannelIndex, t, lDataCount);
                if (lBuffer == NULL)
                {
                    lBuffer = new float[lDataCount * 3];
                    lBufferSize = lDataCount * 3;
                }
                else if (lBufferSize < lDataCount * 3)
                {
                    delete[] lBuffer;
                    lBuffer = new float[lDataCount * 3];
                    lBufferSize = lDataCount * 3;
                }
                else
                    memset(lBuffer, 0, lBufferSize * sizeof(float));

                lCache->Read(lChannelIndex, t, lBuffer, lDataCount);
                if (lChnlInterp == "normals")
                {
                    // display normals cache data
                    // the normal data is per-polygon per-vertex. we can get the polygon vertex index
                    // from the index array of polygon vertex
                    FbxMesh* lMesh = (FbxMesh*)pGeometry;

                    if (lMesh == NULL)
                    {
                        // Only Mesh can have normal cache data
                        continue;
                    }

                    DisplayInt("                Normal Count ", lDataCount);
                    int pi, j, lPolygonCount = lMesh->GetPolygonCount();
                    unsigned lNormalIndex = 0;
                    for (pi = 0; pi < lPolygonCount && lNormalIndex + 2 < lDataCount * 3; pi++)
                    {
                        DisplayInt("                    Polygon ", pi);
                        DisplayString("                    Normals for Each Polygon Vertex: ");
                        int lPolygonSize = lMesh->GetPolygonSize(pi);
                        for (j = 0; j < lPolygonSize && lNormalIndex + 2 < lDataCount * 3; j++)
                        {
                            FbxVector4 normal(lBuffer[lNormalIndex], lBuffer[lNormalIndex + 1], lBuffer[lNormalIndex + 2]);
                            Display3DVector("                       Normal Cache Data  ", normal);
                            lNormalIndex += 3;
                        }
                    }
                }
                else
                {
                    DisplayInt("               Points Count: ", lDataCount);
                    for (unsigned int j = 0; j < lDataCount * 3; j = j + 3)
                    {
                        FbxVector4 points(lBuffer[j], lBuffer[j + 1], lBuffer[j + 2]);
                        Display3DVector("                   Points Cache Data: ", points);
                    }
                }

                lFrame++;
            }

            if (lBuffer != NULL)
            {
                delete[] lBuffer;
                lBuffer = NULL;
            }

            lCache->CloseFile();
        }
    }
}

