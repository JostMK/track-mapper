#include <fbxsdk.h>
#include <iostream>

void AddPlaneWithMaterial(FbxScene *pScene, const std::string &name, const double size) {
    typedef double Vector4[4];
    constexpr Vector4 UP{0,1,0,1};

    const double halfSize = size * .5;
    const Vector4 vertices[4] = {{-halfSize, 0, -halfSize, 1}, {halfSize, 0, -halfSize, 1},
                                 {-halfSize, 0, halfSize, 1},  {halfSize, 0, halfSize, 1}};

    FbxMesh* lMesh = FbxMesh::Create(pScene,"mesh");

    // create control points
    lMesh->InitControlPoints(4);
    FbxVector4* vertex = lMesh->GetControlPoints();
    vertex[0] = vertices[0];
    vertex[1] = vertices[1];
    vertex[2] = vertices[2];
    vertex[3] = vertices[3];

    // create polygons
    lMesh->BeginPolygon(0); // material index
    lMesh->AddPolygon(0);
    lMesh->AddPolygon(1);
    lMesh->AddPolygon(2);
    lMesh->EndPolygon();
    lMesh->BeginPolygon(0); // material index
    lMesh->AddPolygon(1);
    lMesh->AddPolygon(3);
    lMesh->AddPolygon(2);
    lMesh->EndPolygon();

    // specify normals per control point.
    FbxGeometryElementNormal* lNormlElement= lMesh->CreateElementNormal();
    lNormlElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormlElement->SetReferenceMode(FbxGeometryElement::eDirect);

    lNormlElement->GetDirectArray().Add(UP);
    lNormlElement->GetDirectArray().Add(UP);
    lNormlElement->GetDirectArray().Add(UP);
    lNormlElement->GetDirectArray().Add(UP);

    // create material
    FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(pScene, "material");
    lMaterial->Emissive.Set({.0, .0, .0});
    lMaterial->Ambient.Set({1., .0, .0});
    lMaterial->Diffuse.Set({1., .5, .0});
    lMaterial->TransparencyFactor.Set(0.0);
    lMaterial->ShadingModel.Set("Phong");
    lMaterial->Shininess.Set(0.5);

    // add mesh to node and node to scene
    FbxNode* lNode = FbxNode::Create(pScene, name.c_str());
    lNode->SetNodeAttribute(lMesh);
    lNode->AddMaterial(lMaterial);
    pScene->GetRootNode()->AddChild(lNode);
}

int main() {
    std::cout << "Enter fbx file output path: " << std::endl;
    std::string outputPath;
    std::cin >> outputPath;

    FbxManager *lSdkManager = FbxManager::Create();
    FbxScene *lScene = FbxScene::Create(lSdkManager, "PlaneScene");

    AddPlaneWithMaterial(lScene, "plane", 10);

    const auto lExporter = FbxExporter::Create(lSdkManager, "");

    std::cout << "Export as [a]scii or [b]inary format: " << std::endl;
    std::string formatOption;
    std::cin >> formatOption;

    int format;
    if (formatOption[0] == 'a') {
        format = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)");
    } else {
        format = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX binary (*.fbx)");
    }

    if (const bool lExportStatus = lExporter->Initialize(outputPath.c_str(), format, lSdkManager->GetIOSettings());
        !lExportStatus) {
        printf("Call to FbxExporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
    }

    lExporter->Export(lScene);

    lExporter->Destroy();
    lScene->Destroy();

    lSdkManager->Destroy();
    return 0;
}
