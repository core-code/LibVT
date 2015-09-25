#include "LibVT_Internal.h"
#include "LibVT.h"
#include "LibVT_Config.h"

#include <osg/Geode>
#include <osg/TextureRectangle>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#ifdef WIN32
#include <conio.h>
#endif

#define X_RES 1920
#define Y_RES 1200

#ifdef WIN32
#define SHADER_PATH "/Users/clauslokal/Documents/Visual Studio 2005/Projects/VirtualTexturing/LibVT/"
#else
#define SHADER_PATH "VTOSG.app/Contents/Resources/"
#endif

osg::TextureRectangle* texture;
osg::Camera* pre_camera;
osg::Image* image;


osg::StateSet* createSS();
osg::Geode* createVTShapes(osg::StateSet* ss);
osg::Geode* createNonVTShapes(osg::StateSet* ss);
void loadShaderSource(osg::Shader* obj, const std::string& fileName, const std::string& prelude);

class VTExitHandler : public osgGA::GUIEventHandler // TODO: this exit handler never receives the exit events! mac bug or something else?
{
public: 
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                {
					vtShutdown();
                }
                return false;
            }
            case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
            case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
            {
				vtShutdown();
			}
            default: break;
        }
        
        return false;
    }
};

class VTWindowResizeHandler : public osgViewer::WindowSizeHandler
{
public:
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&aa)
    {
		
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::RESIZE):
            {
				int w = (uint16_t)ea.getWindowWidth(), h = (uint16_t)ea.getWindowHeight();
				
				vtReshape(w, h, 0.0, 0.0, 0.0);
				
				texture->setTextureSize(w >> PREPASS_RESOLUTION_REDUCTION_SHIFT, h >> PREPASS_RESOLUTION_REDUCTION_SHIFT);
				image->allocateImage(w >> PREPASS_RESOLUTION_REDUCTION_SHIFT, h >> PREPASS_RESOLUTION_REDUCTION_SHIFT, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
			//	image->setInternalTextureFormat(GL_RGBA);
				pre_camera->setViewport(0, 0, w >> PREPASS_RESOLUTION_REDUCTION_SHIFT, h >> PREPASS_RESOLUTION_REDUCTION_SHIFT);

				
				break;
            }
            default:
                break;
        }
        return WindowSizeHandler::handle(ea, aa);
    }
};


struct PreDrawCallback : public osg::Camera::DrawCallback
{
    virtual void operator () (const osg::Camera&) const
    {
		vtMapNewPages();
    }
};

struct PostDrawCallback : public osg::Camera::DrawCallback
{
    PostDrawCallback(osg::Image* image): _image(image)
    {
    }
	
    virtual void operator () (const osg::Camera&) const
    {
        if (_image && _image->getPixelFormat()==GL_BGRA && _image->getDataType()==GL_UNSIGNED_INT_8_8_8_8_REV)
        {
			//std::string filename = std::string("/Users/julian/Desktop/test.rgb");
			//osgDB::writeImageFile(*_image, filename);
			
			vtExtractNeededPages((const uint32_t *) _image->data());
        }
		else
		{
			printf("houston we have a problem");
		}

    }
    
    osg::Image* _image;
};

osg::Group* createShapes()
{
	osg::Group* group = new osg::Group;
	
	osg::StateSet* ss = createSS();	
	osg::Geode* vtgeode = createVTShapes(ss);
	osg::Geode* nonvtgeode = createNonVTShapes(ss);	
	osg::Group* vtgroup_prerender = new osg::Group;
	osg::Group* vtgroup_mainpass = new osg::Group;	
		
	
	texture = new osg::TextureRectangle;
	texture->setTextureSize(X_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT, Y_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT);
	texture->setInternalFormat(GL_BGRA);
	texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	
	pre_camera = new osg::Camera;
	pre_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	pre_camera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	pre_camera->setViewport(0, 0, X_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT, Y_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT); // TODO: fix the hardcoding
	pre_camera->setRenderOrder(osg::Camera::PRE_RENDER);
	pre_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	
	image = new osg::Image;
	image->allocateImage(X_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT, Y_RES >> PREPASS_RESOLUTION_REDUCTION_SHIFT, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
	image->setInternalTextureFormat(GL_RGBA);
	pre_camera->attach(osg::Camera::COLOR_BUFFER, image, 0, 0);
	pre_camera->setPostDrawCallback(new PostDrawCallback(image));
	texture->setImage(0, image);
	
	
	char *prelude = vtGetShaderPrelude();

	// setup the shaders used for virtual textured objects prepass
	osg::StateSet* vtpreState = vtgroup_prerender->getOrCreateStateSet();
	osg::Program* vtpreProgramObject = new osg::Program;
	osg::Shader* vtpreVertexObject = new osg::Shader( osg::Shader::VERTEX );
	osg::Shader* vtpreFragmentObject = new osg::Shader( osg::Shader::FRAGMENT );
	vtpreProgramObject->addShader( vtpreFragmentObject );
	vtpreProgramObject->addShader( vtpreVertexObject );
	vtpreState->addUniform( new osg::Uniform("pageTableTexture", TEXUNIT_FOR_PAGETABLE) );
	vtpreState->addUniform( new osg::Uniform("mipcalcTexture", TEXUNIT_FOR_MIPCALC) );
	vtpreState->addUniform( new osg::Uniform("mip_bias", vtGetBias()) ); // this shold be done every frame if we want dynamic LoD adjustment

	
	loadShaderSource( vtpreVertexObject, SHADER_PATH "readback.vert", string(prelude));
	loadShaderSource( vtpreFragmentObject, SHADER_PATH "readback.frag", string(prelude));

	vtpreState->setAttributeAndModes(vtpreProgramObject, osg::StateAttribute::ON);
	
	// setup the shaders used for virtual textured objects mainpass
	osg::StateSet* vtmainState = vtgroup_mainpass->getOrCreateStateSet();
	osg::Program* vtmainProgramObject = new osg::Program;
	osg::Shader* vtmainVertexObject = new osg::Shader( osg::Shader::VERTEX );
	osg::Shader* vtmainFragmentObject = new osg::Shader( osg::Shader::FRAGMENT );
	vtmainProgramObject->addShader( vtmainFragmentObject );
	vtmainProgramObject->addShader( vtmainVertexObject );
	vtmainState->addUniform( new osg::Uniform("pageTableTexture", TEXUNIT_FOR_PAGETABLE) ); // TODO: we gotta make sure OSG NEVER USES THESE TEXUNITS!!
	vtmainState->addUniform( new osg::Uniform("physicalTexture", TEXUNIT_FOR_PHYSTEX) );
	vtmainState->addUniform( new osg::Uniform("mip_bias", vtGetBias()) );

	loadShaderSource( vtmainVertexObject, SHADER_PATH "renderVT.vert", string(prelude));
	loadShaderSource( vtmainFragmentObject, SHADER_PATH "renderVT.frag", string(prelude));

	vtmainState->setAttributeAndModes(vtmainProgramObject, osg::StateAttribute::ON); // TODO: barf on shader errors
	
	free(prelude);

	
	
	vtgroup_prerender->addChild(vtgeode);
	vtgroup_mainpass->addChild(vtgeode);

	pre_camera->addChild(vtgroup_prerender);
	
	group->addChild(pre_camera);
	group->addChild(vtgroup_mainpass);
	group->addChild(nonvtgeode);
	
    return group;
}

int main(int, char **)
{
#ifndef WIN32
	vtInit("/Users/julian/Documents/Development/VirtualTexturing/_texdata/32k_b1_jpg", "jpg", 1, 8, 256);
#else
	vtInit("/Users/clauslokal/Documents/Visual Studio 2005/Projects/VirtualTexturing/LibVT-Scripts/_texture_atlas00", "jpg", 0, 6, 256);
#endif
	
    vtReshape(X_RES, Y_RES, 0.0, 0.0, 0.0); // TODO: hardcoding sucks but obtaining it doesn't seem to work at this point

    osgViewer::Viewer viewer;
	
	viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    viewer.setSceneData( createShapes() );
    viewer.addEventHandler(new VTWindowResizeHandler());
    viewer.addEventHandler(new VTExitHandler());
	viewer.getCamera()->setPreDrawCallback(new PreDrawCallback());

	viewer.realize(); // so we have working GL for vtPrepare()
#ifdef WIN32
	// For creating the test scene, the OpenGL context of the viewer is required to be active.
	osgViewer::Viewer::Contexts contexts;
	viewer.getContexts( contexts );
	bool success = contexts[0]->makeCurrent();

	// The OpenGL context is now active.
	HGLRC gl_context = wglGetCurrentContext();
	viewer.setSceneData( createShapes() );
	//init_opengl_function_pointers_win32();

#endif
	vtPrepare((const GLuint) 0, (const GLuint) 0); // we had to set the uniforms ourselves because we passed 0 here. but the real opengl shader ids are kinda hard to obtain

    return viewer.run();
}