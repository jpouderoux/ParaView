/*=========================================================================

   Program: ParaView
   Module:    pqStandardColorButton.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqStandardColorButton.h"

// Server Manager Includes.
#include "vtkSMGlobalPropertiesManager.h"
#include "vtkSMPropertyIterator.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkEventQtSlotConnect.h"

// Qt Includes.
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QActionGroup>
#include <QColorDialog>

// ParaView Includes.
#include "pqApplicationCore.h"
#include "pqSetName.h"

//-----------------------------------------------------------------------------
pqStandardColorButton::pqStandardColorButton(QWidget* _parent) :
  Superclass(_parent)
{
  this->VTKConnect = vtkEventQtSlotConnect::New();
  this->setPopupMode(QToolButton::MenuButtonPopup);
  this->updateMenu();

  pqApplicationCore* core = pqApplicationCore::instance();
  vtkSMProxy* globalProps = core->getGlobalPropertiesManager();

  this->VTKConnect->Connect(
    globalProps, vtkCommand::PropertyModifiedEvent,
    this, SLOT(updateMenu()));
}

//-----------------------------------------------------------------------------
pqStandardColorButton::~pqStandardColorButton()
{
  this->VTKConnect->Delete();
}

//-----------------------------------------------------------------------------
void pqStandardColorButton::setStandardColor(const QString& name)
{
  // user manually changed the color, unset the "standard color" link.
  foreach (QAction* action, this->menu()->actions())
    {
    if (action->isCheckable())
      {
      action->setChecked(
        action->data().toString() == name);
      }
    }
}

//-----------------------------------------------------------------------------
QString pqStandardColorButton::standardColor()
{
  // user manually changed the color, unset the "standard color" link.
  foreach (QAction* action, this->menu()->actions())
    {
    if (action->isCheckable() && action->isChecked())
      {
      return action->data().toString();
      }
    }
  return QString();
}

//-----------------------------------------------------------------------------
void pqStandardColorButton::updateMenu()
{
  QString current_standard_color = this->menu()? this->standardColor() :
    QString();

  // destroy the old one.
  delete this->menu();

  QMenu *popupMenu = new QMenu(this);
  popupMenu << pqSetName("StandardColorMenu");
  this->setMenu(popupMenu);

  QActionGroup * action_group = new QActionGroup(popupMenu);

  QObject::connect(popupMenu, SIGNAL(triggered(QAction*)),
    this, SLOT(actionTriggered(QAction*)));

  pqApplicationCore* core = pqApplicationCore::instance();
  vtkSMProxy* globalProps = core->getGlobalPropertiesManager();
  vtkSMPropertyIterator* iter = globalProps->NewPropertyIterator();
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
    vtkSMDoubleVectorProperty* dvp = vtkSMDoubleVectorProperty::SafeDownCast(
      iter->GetProperty());
    if (dvp && dvp->GetNumberOfElements() == 3)
      {
      QColor qcolor;
      qcolor.setRgbF(dvp->GetElement(0), dvp->GetElement(1), dvp->GetElement(2));

      QAction* action = popupMenu->addAction(
        this->renderColorSwatch(qcolor), dvp->GetXMLLabel());
      action << pqSetName(iter->GetKey());
      action->setData(QVariant(iter->GetKey()));
      action->setCheckable(true);
      action_group->addAction(action);
      }
    }
  iter->Delete();
  this->setStandardColor(current_standard_color);
}

//-----------------------------------------------------------------------------
void pqStandardColorButton::actionTriggered(QAction* action)
{
  QString prop_name = action->data().toString();
  pqApplicationCore* core = pqApplicationCore::instance();
  vtkSMProxy* globalProps = core->getGlobalPropertiesManager();
  vtkSMDoubleVectorProperty* dvp = vtkSMDoubleVectorProperty::SafeDownCast(
    globalProps->GetProperty(prop_name.toLatin1().data()));
  QColor color;
  color.setRgbF(dvp->GetElement(0), dvp->GetElement(1), dvp->GetElement(2));
  emit this->beginUndo(this->UndoLabel);
  this->setChosenColor(color);
  emit this->standardColorChanged(this->standardColor());
  emit this->endUndo();
}

//-----------------------------------------------------------------------------
void pqStandardColorButton::chooseColor()
{
  QColor newColor = QColorDialog::getColor(this->Color, this);
  if (newColor != this->Color)
    {
    emit this->beginUndo(this->UndoLabel);
    this->setChosenColor(newColor);

    // user manually changed the color, unset the "standard color" link.
    foreach (QAction* action, this->menu()->actions())
      {
      if (action->isCheckable())
        {
        action->setChecked(false);
        }
      }
    emit this->standardColorChanged(this->standardColor());
    emit this->endUndo();
    }
}
