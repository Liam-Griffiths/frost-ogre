//
// Frost-Ogre
// 

#include <iostream>
#include "includes/Headers.h"

using namespace Ogre;

int main()
{

	SDL_Init(SDL_INIT_VIDEO);   // Initialize SDL2

	SDL_Window *window;

	// Create an application window with the following settings:
	window = SDL_CreateWindow( 
		"frost-ogre",                  //    window title
		SDL_WINDOWPOS_UNDEFINED,           //    initial x position
		SDL_WINDOWPOS_UNDEFINED,           //    initial y position
		640,                               //    width, in pixels
		480,                               //    height, in pixels
		SDL_WINDOW_SHOWN //    flags - see below
	);

	// Check that the window was successfully made
	if(window==NULL)
	{   
		// In the event that the window could not be made...
		std::cout << "Could not create window: " << SDL_GetError() << '\n';
		return 1;
	}

	Ogre::Root* root = new Ogre::Root("", "", "");
	root->loadPlugin (OGRE_PLUGIN_DIR_REL + std::string("/RenderSystem_GL"));
	root->loadPlugin (OGRE_PLUGIN_DIR_REL + std::string("/Plugin_OctreeSceneManager"));
	root->setRenderSystem(root->getRenderSystemByName("OpenGL Rendering Subsystem"));

	root->initialise(false);


	//get the native whnd
	struct SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

	if (SDL_GetWindowWMInfo(window, &wmInfo) == SDL_FALSE)
	{
		throw std::runtime_error("Couldn't get WM Info!");
	}

	Ogre::String winHandle;

	switch (wmInfo.subsystem)
	{
		case SDL_SYSWM_X11:
			winHandle = Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.window);
			break;
		default:
			throw std::runtime_error("Unexpected WM!");
			break;
	}

	Ogre::NameValuePairList params;
	params.insert(std::make_pair("title", "frost-ogre"));
	params.insert(std::make_pair("FSAA", "0"));
	params.insert(std::make_pair("vsync", "false"));
	params.insert(std::make_pair("parentWindowHandle",  winHandle));


	Ogre::RenderWindow* ogreWindow = Ogre::Root::getSingleton().createRenderWindow("frost-ogre", 640, 480, false, &params);
	ogreWindow->setVisible(true);

	// setup the default base scene manager -- sufficient for our purposes
	SceneManager* sceneMgr = root->createSceneManager("OctreeSceneManager", "MainManager");

	ResourceGroupManager * assets = ResourceGroupManager::getSingletonPtr();
	assets->addResourceLocation("../assets/meshes","FileSystem");
	assets->addResourceLocation("../assets/materials","FileSystem");
	assets->addResourceLocation("../assets/materials/scripts","FileSystem");
	assets->addResourceLocation("../assets/materials/textures","FileSystem");

	assets->addResourceLocation("assets/meshes","FileSystem");
	assets->addResourceLocation("assets/materials","FileSystem");
	assets->addResourceLocation("assets/materials/scripts","FileSystem");
	assets->addResourceLocation("assets/materials/textures","FileSystem");

	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
	//there are two kinds of shadows, texture shadows and stencil shadows
	//more about this later
	sceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	sceneMgr->setShadowTextureSize(1024);
	
	//sceneMgr->setSkyBox(true, "Examples/SceneSkyBox1",500);
	sceneMgr->setAmbientLight(ColourValue(1,1,1));

	
	//create directional light for terrain light map
	Ogre::Vector3 lightdir(0.55, -0.3, -0.75);
	lightdir.normalise();

	Ogre::Light* light = sceneMgr->createLight("testLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));

	sceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));


	//create 'player node' to position the player character
	Ogre::SceneNode * mPlayerNode = sceneMgr->getRootSceneNode()->createChildSceneNode("PlayerNode");
	//set initial position for player
	mPlayerNode->translate(0,0,0);
	//rotate it about a vertical axis if desired
	mPlayerNode->rotate(Vector3::UNIT_Y,Ogre::Degree(180),Ogre::Node::TS_LOCAL);
	//load model
	Entity* ninjaMesh = sceneMgr->createEntity("ninja", "ninja.mesh");
	//attach model to mPlayerNode 
	mPlayerNode->attachObject(ninjaMesh);
	//set player to cast shadows
	ninjaMesh->setCastShadows(true);

	// create a single camera, and a viewport that takes up the whole window (default behavior)
	Camera *camera = sceneMgr->createCamera("MainCam");
	//camera->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);

	Viewport *vp = ogreWindow->addViewport(camera);
	vp->setDimensions(0.0f, 0.0f, 1.0f, 1.0f);
	camera->setAspectRatio((float)vp->getActualWidth() / (float) vp->getActualHeight());
	camera->setFarClipDistance(1500.0f);
	camera->setNearClipDistance(5.0f);
	
	//create a node for the camera
	SceneNode* cameraNode = sceneMgr->getRootSceneNode()->createChildSceneNode("CameraNode");
	//attach the camera to it
	cameraNode->attachObject(camera);
	//move it into position
	cameraNode->translate(0,300,300);
	//rotate it downwards about a horizontal axis
	cameraNode->rotate(Ogre::Vector3::UNIT_X,Ogre::Degree(-30));


	//create a plane to act as a backdrop
	//next week we will replace this with terrain
	Ogre::MovablePlane* pPlane;
	Ogre::Entity* pPlaneEnt;
	Ogre::SceneNode* pPlaneNode;
	pPlane = new Ogre::MovablePlane("Plane");
	pPlane->d = 0;
	pPlane->normal = Ogre::Vector3::UNIT_Y;
	Ogre::MeshManager::getSingleton().createPlane("PlaneMesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *pPlane, 500, 500, 32,32, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
	pPlaneEnt = sceneMgr->createEntity("PlaneEntity", "PlaneMesh");
	pPlaneEnt->setMaterialName("Ogre/Terrain");
	pPlaneEnt->setCastShadows(false);
	pPlaneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
	pPlaneNode->attachObject(pPlaneEnt);

	while(1)
	{
		root->renderOneFrame();
		
	    SDL_Event event;
	    if(SDL_PollEvent(&event))
	    {
			if (event.type == SDL_QUIT)
			{
				break;
			}
	    }

	}

	delete root;

  
	// Close and destroy the window
	SDL_DestroyWindow(window); 

	// Clean up
	SDL_Quit();

	return 0; 
}
