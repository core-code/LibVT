//
//  SDLOpenGLView.m
//  Core3D
//
//  Created by Julian Mayer on 11.01.08.
//  Copyright 2008 - 2010 A. Julian Mayer.
//
/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "SDL.h"
#include "Core3D.h"
#include "Simulation.h"

#define PRINT_SIMPLE_FPS 1

void handleKey(int key, BOOL down);

int main(int argc, char *argv[])
{
	int i, disablevbl = 0;
	uint32_t startticks;

	for(i=1; i < argc; i++)
	{
		if (*(argv[i]+1) == 'v')
			disablevbl = atoi(argv[i]+2);
		else if (*(argv[i]+1) == 'b')
			globalSettings.doBenchmark = 1;
	}

	int w,h;
	const SDL_VideoInfo *info = NULL;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

#ifdef WIN32
	NSInitializeProcess(argc,(const char **)argv);
	[NSApp finishLaunching];
#endif

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
		fatal("Error: video initialization failed: %s\n", SDL_GetError());

	info = SDL_GetVideoInfo();
	if(!info)
		fatal("Error: video query failed: %s\n", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

//	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"enableFSAA"])
//	{
//		printf("Notice: enabling FSAA: %i\n", [[NSUserDefaults standardUserDefaults] integerForKey:@"fsaa"]);
//		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
//		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, [[NSUserDefaults standardUserDefaults] integerForKey:@"fsaa"]);
//	}

	if (!disablevbl && [[NSUserDefaults standardUserDefaults] boolForKey:@"disableVBL"])
		disablevbl = 0;

	if (!disablevbl)
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	printf("Info: VBL %s!\n", disablevbl ? "disabled" : "enabled");

	pressedKeys = [[NSMutableArray alloc] initWithCapacity:5];

	w = [[[[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"resolution" ofType:@"txt"]] componentsSeparatedByString:@"x"] objectAtIndex:0] intValue];
	h = [[[[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"resolution" ofType:@"txt"]] componentsSeparatedByString:@"x"] objectAtIndex:1] intValue];

	//if(SDL_SetVideoMode(1280, 960, info->vfmt->BitsPerPixel, SDL_OPENGL) == 0)
	if(SDL_SetVideoMode(w ? w : info->current_w, h ? h : info->current_h, info->vfmt->BitsPerPixel, SDL_OPENGL | SDL_FULLSCREEN) == 0)
	       fatal("Error: video mode set failed: %s\n", SDL_GetError());

 //   SDL_ShowCursor(SDL_DISABLE);

	Scene *scene = [Scene sharedScene];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	[scene setSimulator:[[[NSClassFromString([[NSBundle mainBundle] objectForInfoDictionaryKey:@"SimulationClass"]) alloc] init] autorelease]];
	[scene reshape:[NSArray arrayWithObjects:[NSNumber numberWithInt:(w ? w : info->current_w)], [NSNumber numberWithInt:h ? h : info->current_h], nil]];

	startticks = SDL_GetTicks();

    while(1)
	{
		if (1)// (globalSettings.displayFPS || globalSettings.doBenchmark)
		{
			if ((globalSettings.doBenchmark) && (globalInfo.frame == 60 * 40))
				fatal("finished benchmark");

			static int frames = 0;
			static int lastmillis = 0;

			int millis = SDL_GetTicks();

#ifndef PRINT_FRAMERATE_EVERY_60_FRAMES
			if (((millis - lastmillis) > 1000) && (globalInfo.fps = frames))
#else
			if ((frames >= 60) && (globalInfo.fps = ((1000.0 / (float)(millis - lastmillis)) * 60.0)))
#endif
			{
				lastmillis = millis;
				frames = 0;

#ifdef PRINT_DETAILED_STATISTICS
				printf("FPS: %i\tRenderedFaces: %i\nMainRenderPass:\t\t\t RenderedFaces: %i of %i (%.2f%%) VisitedNodes: %i of %i (%.2f%%)\nShadowRenderPass:\t\t\t RenderedFaces: %i of %i (%.2f%%) VisitedNodes: %i of %i (%.2f%%)\nAdditionalRenderPass:\t RenderedFaces: %i of %i (%.2f%%) VisitedNodes: %i of %i (%.2f%%) DrawCalls: %i\n", (int)globalInfo.fps, globalInfo.renderedFaces[0]+globalInfo.renderedFaces[1]+globalInfo.renderedFaces[2], globalInfo.renderedFaces[0], globalInfo.totalFaces, (float)globalInfo.renderedFaces[0]*100/(float)globalInfo.totalFaces, globalInfo.visitedNodes[0], globalInfo.totalNodes, (float)globalInfo.visitedNodes[0]*100/(float)globalInfo.totalNodes, globalInfo.renderedFaces[1], globalInfo.totalFaces, (float)globalInfo.renderedFaces[1]*100/(float)globalInfo.totalFaces, globalInfo.visitedNodes[1], globalInfo.totalNodes, (float)globalInfo.visitedNodes[1]*100/(float)globalInfo.totalNodes, globalInfo.renderedFaces[2], globalInfo.totalFaces, (float)globalInfo.renderedFaces[2]*100/(float)globalInfo.totalFaces, globalInfo.visitedNodes[2], globalInfo.totalNodes, (float)globalInfo.visitedNodes[2]*100/(float)globalInfo.totalNodes, globalInfo.drawCalls);
#elif PRINT_SIMPLE_FPS
				printf("FPS: %i\tRenderedFaces: %i\n", (int)globalInfo.fps, globalInfo.renderedFaces);
#endif
			}
			else
				frames++;
		}
		else
		{
			static int totalmillis = 0, lastmillis = 0, curtime;
			int millis = SDL_GetTicks();

			if(millis<totalmillis) millis = totalmillis;
			static int fpserror = 0;
			int delay = 1000/60 - (millis-totalmillis);
			if(delay < 0) fpserror = 0;
			else
			{
				fpserror += 1000%60;
				if(fpserror >= 60)
				{
					++delay;
					fpserror -= 60;
				}
				if(delay > 0)
				{
					SDL_Delay(delay);
					millis += delay;
				}
			}
			int elapsed = millis-totalmillis;
			curtime = elapsed;
			lastmillis += curtime;
			totalmillis = millis;
		}

	//	while (globalInfo.frame < ((SDL_GetTicks() - startticks) / 16.666666))			// fuck "framerateunabhängige Spiellogik"
			[scene update];

		[scene render];
		SDL_GL_SwapBuffers();

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					handleKey(event.key.keysym.sym, YES);
					break;
				case SDL_KEYUP:
					handleKey(event.key.keysym.sym, NO);
					break;
				case SDL_QUIT:
					fatal("shutting down normally");
					break;
				case SDL_MOUSEMOTION:
					if (SDL_BUTTON(1) == event.motion.state)
						if ([[scene simulator] respondsToSelector:@selector(mouseDragged:withFlags:)])
							[(Simulation *)[scene simulator] mouseDragged:vector2f(event.motion.xrel, event.motion.yrel) withFlags:0];
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_WHEELUP)
					{
						if ([[scene simulator] respondsToSelector:@selector(scrollWheel:)])
							[(Simulation *)[scene simulator] scrollWheel:3.0];
					}
					else if(event.button.button == SDL_BUTTON_WHEELDOWN)
					{
						if ([[scene simulator] respondsToSelector:@selector(scrollWheel:)])
							[(Simulation *)[scene simulator] scrollWheel:-3.0];
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT)
						if ([[scene simulator] respondsToSelector:@selector(rightMouseUp:)])
							[(Simulation *)[scene simulator] rightMouseUp:nil];
					break;
			}
		}
    }

	[pool release];

    return 0;
}

void vtShutdown();

void handleKey(int key, BOOL down)
{
	NSString *str = nil;

	switch(key)
	{
		case SDLK_ESCAPE:
			vtShutdown();

			fatal("shutting down normally");
			break;
		case SDLK_TAB:
			str = [[NSNumber numberWithUnsignedInt:9] stringValue];
			break;
		case SDLK_UP:
			str = [[NSNumber numberWithUnsignedInt:NSUpArrowFunctionKey] stringValue];
			break;
		case SDLK_DOWN:
			str = [[NSNumber numberWithUnsignedInt:NSDownArrowFunctionKey] stringValue];
			break;
		case SDLK_LEFT:
			str = [[NSNumber numberWithUnsignedInt:NSLeftArrowFunctionKey] stringValue];
			break;
		case SDLK_RIGHT:
			str = [[NSNumber numberWithUnsignedInt:NSRightArrowFunctionKey] stringValue];
			break;
		case SDLK_KP8:
			str = [[NSNumber numberWithUnsignedInt:NSUpArrowFunctionKey] stringValue];
			break;
		case SDLK_KP4:
			str = [[NSNumber numberWithUnsignedInt:NSLeftArrowFunctionKey] stringValue];
			break;
		case SDLK_KP6:
			str = [[NSNumber numberWithUnsignedInt:NSRightArrowFunctionKey] stringValue];
			break;
		case SDLK_KP_PLUS:
			str = [[NSNumber numberWithUnsignedInt:'+'] stringValue];
			break;
		case SDLK_KP_MINUS:
			str = [[NSNumber numberWithUnsignedInt:'-'] stringValue];
			break;
		case SDLK_F1...SDLK_F15:
			str = [[NSNumber numberWithUnsignedInt:NSF1FunctionKey + key - SDLK_F1] stringValue];
			break;
		default:
			str = [[NSNumber numberWithUnsignedInt:key] stringValue];
			break;
	}

	if (str)
		down ? [pressedKeys addObject:str] : [pressedKeys removeObject:str];
}
