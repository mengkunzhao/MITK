#include "mitkVtkLayerController.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <algorithm>

mitk::VtkLayerController::VtkLayerController(vtkRenderWindow* renderWindow)
{
  m_RenderWindow = renderWindow;
  m_BackgroundRenderers.clear();
  m_ForegroundRenderers.clear();
  m_SceneRenderers.clear();

}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the background.
 * With forceAbsoluteBackground set true a renderer can be placed at the absolute background of the scene.
 * Multiple calls with forceAbsoluteBackground set true will set the latest registered renderer as background.
 */
void mitk::VtkLayerController::InsertBackgroundRenderer(vtkRenderer* renderer, bool forceAbsoluteBackground)
{
  
  if(renderer == NULL)
    return;
  
  // Remove renderer if it already exists 
  RemoveRenderer(renderer);

  if(forceAbsoluteBackground)
  {
    RendererVectorType::iterator it = m_BackgroundRenderers.begin();
    m_BackgroundRenderers.insert(it,renderer);
  }
  else
    m_BackgroundRenderers.push_back(renderer);
  UpdateLayers();
}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the foreground.
 * With forceAbsoluteBackground set true a renderer can be placed at the absolute foreground of the scene.
 * Multiple calls with forceAbsoluteForeground set true will set the latest registered renderer as foreground.
 */
void mitk::VtkLayerController::InsertForegroundRenderer(vtkRenderer* renderer, bool forceAbsoluteForeground)
{
  
  if(renderer == NULL)
    return;
  
  // Remove renderer if it already exists 
  RemoveRenderer(renderer);

  if(forceAbsoluteForeground)
  {
    RendererVectorType::iterator it = m_ForegroundRenderers.begin();
    m_ForegroundRenderers.insert(it,renderer);
  }
  else
    m_ForegroundRenderers.push_back(renderer);

  UpdateLayers();
}
/**
 * Returns the Scene Renderer
 */
vtkRenderer* mitk::VtkLayerController::GetSceneRenderer()
{
   RendererVectorType::iterator it = m_SceneRenderers.begin();
   return (*it);
}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered between background renderers and
 * foreground renderers.
 */
void mitk::VtkLayerController::InsertSceneRenderer(vtkRenderer* renderer)
{
  
  if(renderer == NULL)
    return;
  
  // Remove renderer if it already exists
  RemoveRenderer(renderer);

  m_SceneRenderers.push_back(renderer);
  UpdateLayers();
}
/**
 * A renderer which has been inserted via a insert... function can be removed from the vtkRenderWindow with
 * RemoveRenderer.
 */
void mitk::VtkLayerController::RemoveRenderer(vtkRenderer* renderer)
{
  RendererVectorType::iterator it;
  // background layers
  if(m_BackgroundRenderers.size() > 0)
  {
    it = std::find(m_BackgroundRenderers.begin(),m_BackgroundRenderers.end(),renderer);
    if(it != m_BackgroundRenderers.end())
    {
      m_BackgroundRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
  // scene layers
  if(m_SceneRenderers.size() > 0)
  {
    it = std::find(m_SceneRenderers.begin(),m_SceneRenderers.end(),renderer);
    if(it != m_SceneRenderers.end())
    {
      m_SceneRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
  // foreground layers
  if(m_ForegroundRenderers.size() > 0 )
  {
    it = std::find(m_ForegroundRenderers.begin(),m_ForegroundRenderers.end(),renderer);
    if(it != m_ForegroundRenderers.end())
    {
      m_ForegroundRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
}
/**
 * Connects a VtkRenderWindow with the layer controller.
 */
void mitk::VtkLayerController::SetRenderWindow(vtkRenderWindow* renwin)
{
  if(renwin != NULL)
  {
    RendererVectorType::iterator it;
    // Tell all renderers that there is a new renderwindow
    for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    // Set the new RenderWindow
    m_RenderWindow = renwin;
  }
  // Now sort renderers and add them to the renderwindow
  UpdateLayers();
}

/**
* Returns true if a renderer has been inserted 
*/
bool mitk::VtkLayerController::IsRendererInserted(vtkRenderer* renderer)
{
  RendererVectorType::iterator it;
  // background layers
  if(m_BackgroundRenderers.size() > 0)
  {
    it = std::find(m_BackgroundRenderers.begin(),m_BackgroundRenderers.end(),renderer);
    if((*it) == renderer)
    {
      return true;
    }
  }
  // scene layers
  if(m_SceneRenderers.size() > 0)
  {
    it = std::find(m_SceneRenderers.begin(),m_SceneRenderers.end(),renderer);
    if((*it) == renderer)
    {
      return true;
    }
  }
  // foreground layers
  if(m_ForegroundRenderers.size() > 0 )
  {
    it = std::find(m_ForegroundRenderers.begin(),m_ForegroundRenderers.end(),renderer);
    if((*it) == renderer)
    {
      return true;
    }
  }
  return false;
  
}
/**
 * Internally used to sort all registered renderers and to connect the with the vtkRenderWindow.
 * Mention that VTK Version 5 and above is rendering higher numbers in the background and VTK
 * Verison < 5 in the foreground.
 */
void mitk::VtkLayerController::UpdateLayers()
{
  // Remove all Renderers from renderwindow
  vtkRendererCollection* v = m_RenderWindow->GetRenderers();
  v->RemoveAllItems();


  int numberOfLayers = m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size();
  int currentLayerNumber;
  bool traverseUpwards;

  #if ( VTK_MAJOR_VERSION >= 5 )
    currentLayerNumber  = 0;
    traverseUpwards     = true;
  #else
    currentLayerNumber  = numberOfLayers - 1;
    traverseUpwards     = false;
  #endif
  
  
  m_RenderWindow->SetNumberOfLayers(numberOfLayers);
  RendererVectorType::iterator it;
  // assign a layer number for the backround renderers
  for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
  {
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));
    
    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
  // assign a layer number for the scene renderers
  for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
  {   
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));
    
    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
  // assign a layer number for the foreground renderers
  for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
  {
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));
    
    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
}
/**
 * Returns the number of renderers in the renderwindow.
 */
int mitk::VtkLayerController::GetNumberOfRenderers()
{
  return m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size();
}

