#include <QObject>
#include <QtQml>
#include <qqmlwebchannel.h>

#include "ComponentManager.h"

#include "power/PowerComponent.h"
#include "server/HTTPServer.h"
#include "input/InputComponent.h"
#include "player/PlayerComponent.h"
#include "display/DisplayComponent.h"
#include "system/SystemComponent.h"
#include "system/UpdaterComponent.h"
#include "settings/SettingsComponent.h"
#include "remote/RemoteComponent.h"

#include "server/HTTPServer.h"

#if KONVERGO_OPENELEC
#include "system/openelec/OESystemComponent.h"
#include "system/ConnmanComponent.h"
#endif

#include "QsLog.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
ComponentManager::ComponentManager() : QObject(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentManager::registerComponent(ComponentBase* comp)
{
  if (m_components.contains(comp->componentName()))
  {
    QLOG_ERROR() << "Component" << comp->componentName() << "already registered!";
    return;
  }
  
  if (comp->componentInitialize())
  {
    QLOG_INFO() << "Component:" << comp->componentName() << "inited";
    m_components[comp->componentName()] = comp;

    // define component as property for qml
    m_qmlProperyMap.insert(comp->componentName(), QVariant::fromValue(comp));
  }
  else
  {
    QLOG_ERROR() << "Failed to init component:" << comp->componentName();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentManager::initialize()
{
  // then settings, since all other components
  // might have some settings
  //
  registerComponent(&SettingsComponent::Get());

  // start our web server
  auto server = new HttpServer(this);
  server->start();

  registerComponent(&InputComponent::Get());
  registerComponent(&SystemComponent::Get());
  registerComponent(&DisplayComponent::Get());
  registerComponent(&UpdaterComponent::Get());
  registerComponent(&RemoteComponent::Get());
  registerComponent(&PlayerComponent::Get());
  registerComponent(&PowerComponent::Get());

#if KONVERGO_OPENELEC
  registerComponent(&OESystemComponent::Get());
  registerComponent(&ConnmanComponent::Get());
#endif

  for(ComponentBase* component : m_components.values())
    component->componentPostInitialize();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ComponentManager::setWebChannel(QWebChannel* webChannel)
{
  for(ComponentBase* comp : m_components.values())
  {
    if (comp->componentExport())
    {
      QLOG_DEBUG() << "Adding component:" << comp->componentName() << "to webchannel";
      webChannel->registerObject(comp->componentName(), comp);
    }
  }
}
