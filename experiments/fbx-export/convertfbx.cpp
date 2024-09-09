#include <iostream>
#include <fbxsdk.h>

int main() {
    std::cout << "Enter path to fbx file: " << std::endl;
    std::string lFilename;
    std::cin >> lFilename;

    FbxManager *lSdkManager = FbxManager::Create();

    FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    FbxImporter *lImporter = FbxImporter::Create(lSdkManager, "");
    if (!lImporter->Initialize(lFilename.c_str(), -1, lSdkManager->GetIOSettings())) {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }

    FbxScene *lScene = FbxScene::Create(lSdkManager, "ConvertScene");
    lImporter->Import(lScene);
    lImporter->Destroy();

    const auto lExporter = FbxExporter::Create(lSdkManager, "");
    const int lAsciiFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)");
    const std::string outfileName = lFilename.substr(0,lFilename.size()-4) + "-ascii.fbx";
    if(const bool lExportStatus = lExporter->Initialize(outfileName.c_str(), lAsciiFormat, lSdkManager->GetIOSettings()); !lExportStatus) {
        printf("Call to FbxExporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
    }

    lExporter->Export(lScene);

    lExporter->Destroy();
    lScene->Destroy();
    lSdkManager->Destroy();
    return 0;
}

