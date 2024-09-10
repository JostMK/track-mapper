//
// Created by Jost on 05/09/2024.
//

#include "TrackScene.h"

#include <fbxsdk.h>

namespace TrackMapper::Scene {
    void AddMeshToScene(const Mesh &mesh, FbxScene *pScene, FbxSurfacePhong *lMaterial);
    FbxSurfacePhong *CreateMaterial(const std::string &name, const Double3 &color, FbxScene *pScene);

    void TrackScene::AddGrassMesh(Mesh &mesh, const bool hasCollision) {
        if (hasCollision) {
            mesh.name = mPhysicsCounter + "GRASS";
            mPhysicsCounter++;
        } else {
            mesh.name = "0GRASS_" + std::to_string(mGrassCounter);
            mGrassCounter++;
        }

        mGrassMeshes.push_back(mesh);
    }

    void TrackScene::AddRoadMesh(Mesh &mesh) {
        mesh.name = std::to_string(mPhysicsCounter) + "ROAD";
        mPhysicsCounter++;

        mRoadMeshes.push_back(mesh);
    }

    void TrackScene::AddSpawnPoint(const std::string& name, const Double3 &position, const Double3 &direction) {
        mEmptyMeshes.emplace_back(name, position, direction);
    }

    void TrackScene::Export(const std::string &filePath, const bool asciiFormat) const {
        // create FBX scene
        auto *lSdkManager = FbxManager::Create();
        auto *lScene = FbxScene::Create(lSdkManager, "ACTrack");

        // populate FBX scene
        const auto grassMat = CreateMaterial("GrassMat", {.47f, .63f, .29f}, lScene);
        for (const Mesh &mesh: mGrassMeshes) {
            AddMeshToScene(mesh, lScene, grassMat);
        }

        const auto roadMat = CreateMaterial("RoadMat", {.28f, .31f, .34f}, lScene);
        for (const Mesh &mesh: mRoadMeshes) {
            AddMeshToScene(mesh, lScene, roadMat);
        }

        //const auto nullMat = CreateMaterial("NULL", {0, 0, 0}, lScene);
        //nullMat->TransparencyFactor.Set(1);
        for (const Mesh &mesh: mEmptyMeshes) {
            FbxNode *lNode = FbxNode::Create(lScene, mesh.name.c_str());
            //lNode->AddMaterial(nullMat);
            lNode->LclTranslation.Set(FbxDouble3{mesh.origin.x, mesh.origin.y, mesh.origin.z});

            FbxVector4 angles;
            FbxVector4 normForward{0,0,1,1};
            FbxVector4 meshForward{mesh.forward.x, mesh.forward.y, mesh.forward.z,1};
            FbxVector4::AxisAlignmentInEulerAngle({0,0,0,1}, meshForward, normForward, angles);
            lNode->LclRotation.Set(FbxDouble3{angles[0], angles[1], angles[2]});

            lScene->GetRootNode()->AddChild(lNode);
        }

        // create FBX exporter
        const auto lExporter = FbxExporter::Create(lSdkManager, "Exporter");
        const auto formatDesc = asciiFormat ? "FBX ascii (*.fbx)" : "FBX binary (*.fbx)";
        const auto format = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription(formatDesc);
        if (const bool lExportStatus = lExporter->Initialize(filePath.c_str(), format, lSdkManager->GetIOSettings());
            !lExportStatus) {
            // TODO: handle error - maybe add project wide logger
            printf("Call to FbxExporter::Initialize() failed.\n");
            printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        }

        // export scene
        lExporter->Export(lScene);

        // clean up
        lExporter->Destroy();
        lScene->Destroy();
        lSdkManager->Destroy();
    }

    void AddMeshToScene(const Mesh &mesh, FbxScene *pScene, FbxSurfacePhong *lMaterial) {
        FbxMesh *lMesh = FbxMesh::Create(pScene, "mesh");

        const int vertexCount = static_cast<int>(mesh.vertices.size()); // size should always be below 40k
        // creating control points
        lMesh->InitControlPoints(vertexCount);
        FbxVector4 *vertex = lMesh->GetControlPoints();
        for (int i = 0; i < vertexCount; ++i) {
            const Double3 p = mesh.vertices[i].position;
            vertex[i] = FbxVector4{p.x, p.y, p.z};
        }

        // specify normals per control point.
        FbxGeometryElementNormal *lNormlElement = lMesh->CreateElementNormal();
        lNormlElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
        lNormlElement->SetReferenceMode(FbxGeometryElement::eDirect);
        // auto normal = lNormlElement->GetDirectArray();
        // normal.Resize(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            const Double3 n = mesh.vertices[i].normal;
            // vertex[i] = FbxVector4{n.x, n.y, n.z};
            lNormlElement->GetDirectArray().Add(FbxVector4{n.x, n.y, n.z});
        }

        // creating polygons
        for (int i = 0; i < mesh.triangles.size(); i += 3) {
            lMesh->BeginPolygon(0); // material index - only one material supported for now
            lMesh->AddPolygon(mesh.triangles[i]);
            lMesh->AddPolygon(mesh.triangles[i + 1]);
            lMesh->AddPolygon(mesh.triangles[i + 2]);
            lMesh->EndPolygon();
        }

        // adding mesh to node and node to scene
        FbxNode *lNode = FbxNode::Create(pScene, mesh.name.c_str());
        lNode->SetNodeAttribute(lMesh);
        lNode->AddMaterial(lMaterial);
        lNode->LclTranslation.Set(FbxDouble3{mesh.origin.x, mesh.origin.y, mesh.origin.z});

        pScene->GetRootNode()->AddChild(lNode);
    }

    FbxSurfacePhong *CreateMaterial(const std::string &name, const Double3 &color, FbxScene *pScene) {
        // creating material
        FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(pScene, name.c_str());
        lMaterial->Emissive.Set({.0, .0, .0});
        lMaterial->Ambient.Set({.0, .0, .0});
        lMaterial->Diffuse.Set({color.x, color.y, color.z});
        lMaterial->TransparencyFactor.Set(.0);
        lMaterial->ShadingModel.Set("Phong");
        lMaterial->Shininess.Set(.0);

        return lMaterial;
    }
} // namespace TrackMapper::Scene
