#pragma once

#include <iostream>
#include <stdexcept>

#include "application.h"

using namespace Engine;

int main(int argc, char* argv[])
{
    // ��ʼ������
    Application app;
    app.create(argc, argv);

    Scene* scene = app.getScene();

    Camera* camera = scene->addCamera(glm::radians(90.0f), 0.1f, 1000.0f);
    scene->setActiveCamera(camera);
    camera->translate(glm::vec3(1.5, 1.0, 2.0));
    camera->rotate(120.0, -10.0);

    // �ĸ������õ���պ�
    //SkyBox *skybox = scene->addSkyBox("Canyon/", "Unfiltered_HDR.dds", "Diffuse_HDR.dds", "Specular_HDR.dds", "FSchlick_DGGX_GSmith.dds");
    //SkyBox *skybox = scene->addSkyBox("Factory/", "Unfiltered_HDR.dds", "Diffuse_HDR.dds", "Specular_HDR.dds", "FSchlick_DGGX_GSmith.dds");
    SkyBox* skybox = scene->addSkyBox("MonValley/", "Unfiltered_HDR.dds", "Diffuse_HDR.dds", "Specular_HDR.dds", "FSchlick_DGGX_GSmith.dds");
    //SkyBox *skybox = scene->addSkyBox("PaperMill/", "Unfiltered_HDR.dds", "Diffuse_HDR.dds", "Specular_HDR.dds", "FSchlick_DGGX_GSmith.dds");
    scene->setActiveSkyBox(skybox);

    // �����õĹ�Դ
    /* Light *light = scene->addLight(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), 2.0f);
     light->translate(glm::vec3(0.0f, 1.5f, 0.0f));

     Light* light1 = scene->addLight(glm::vec4(2.0f, 1.0f, 1.0f, 0.0f), 2.0f);
     light1->translate(glm::vec3(0.0f, 1.5f, 0.0f));*/

     // ģ��

     /*Model *model = scene->addModel("sponza/", "sponza.obj", "phong");
     model->scale(glm::vec3(0.01f, 0.01f, 0.01f));*/

     // ��
    //Model *model = scene->addModel("primitives/", "sphere.obj", "phong");
    //model->scale(glm::vec3(0.01f, 0.01f, 0.01f));

    // ǹ
    //Model* gun = scene->addModel("9mm_Pistol/", "9mm_Pistol.obj", "PBR_IBL");
    //gun->translate(glm::vec3(1.0f, 0.0f, 0.0f));

    // ����
    Model* gun = scene->addModel("hudie/", "hudie.obj", "PBR_IBL");
    gun->translate(glm::vec3(1.0f, 0.0f, 0.0f));

    // ǹ
    // Model *cerberus = scene->addModel("cerberus/", "cerberus.obj", "PBR_IBL");
    // cerberus->translate(glm::vec3(-1.0f, 0.0f, 0.0f));

    app.beginMainLoop();
    app.shutDown();

    return EXIT_SUCCESS;
}