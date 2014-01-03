/*=========================================================================

   Program: ParaView
   Module:  pqSeriesEditorPropertyWidget.cxx

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
#include "pqSeriesEditorPropertyWidget.h"
#include "ui_pqSeriesEditorPropertyWidget.h"

#include "pqPropertiesPanel.h"
#include "vtkPVXMLElement.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMProperty.h"

#include <QAbstractTableModel>
#include <QByteArray>
#include <QColorDialog>
#include <QDataStream>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMimeData>
#include <QPair>
#include <QVector>
#include <QSortFilterProxyModel>

#include <assert.h>

class pqSeriesParametersModel : public QAbstractTableModel
{
  typedef QAbstractTableModel Superclass;

  bool SupportsReorder;
  QIcon MissingColorIcon;
  QVector<QPair<QString, bool> > Visibilities;
  QMap<QString, QString> Labels;
  QMap<QString, QColor> Colors;

  QColor seriesColor(const QModelIndex& idx) const
    {
    if (idx.isValid() && (idx.row() < this->Visibilities.size()) &&
      this->Colors.contains(this->Visibilities[idx.row()].first))
      {
      return this->Colors[this->Visibilities[idx.row()].first];
      }
    return QColor();
    }
public:
  pqSeriesParametersModel(bool supportsReorder, QObject* parentObject=0):
    Superclass(parentObject),
    SupportsReorder(supportsReorder),
    MissingColorIcon(":/pqWidgets/Icons/pqUnknownData16.png")
    {
    this->Visibilities.push_back(QPair<QString, bool>("Alpha", true));
    this->Visibilities.push_back(QPair<QString, bool>("Beta", false));
    }
  virtual ~pqSeriesParametersModel()
    {
    }

  enum ColumnRoles
    {
    VISIBILITY = 0,
    COLOR = 1,
    LABEL = 2
    };

  virtual Qt::ItemFlags flags(const QModelIndex &idx) const
    {
    Qt::ItemFlags value = this->Superclass::flags(idx);
    if (this->SupportsReorder)
      {
      value |= Qt::ItemIsDropEnabled;
      }
    if (idx.isValid())
      {
      value |= Qt::ItemIsDragEnabled;
      switch (idx.column())
        {
      case VISIBILITY:
        return value | Qt::ItemIsUserCheckable;
      case LABEL:
        return value | Qt::ItemIsEditable;
      case COLOR:
      default:
        break;
        }
      }
    return value;
    }

  virtual int rowCount(const QModelIndex& idx=QModelIndex()) const
    {
    Q_UNUSED(idx);
    return this->Visibilities.size();
    }

  virtual int columnCount(const QModelIndex& idx=QModelIndex()) const
    {
    Q_UNUSED(idx);
    return 3;
    }

  virtual QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const
    {
    assert(idx.row() < this->Visibilities.size());

    if (idx.column() == VISIBILITY)
      {
      switch (role)
        {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::StatusTipRole:
        return this->Visibilities[idx.row()].first;

      case Qt::CheckStateRole:
        return this->Visibilities[idx.row()].second?
          Qt::Checked : Qt::Unchecked;
        }
      }
    else if (idx.column() == COLOR)
      {
      QColor color;
      switch (role)
        {
      case Qt::DecorationRole:
        color = this->seriesColor(idx);
        return color.isValid() ? QVariant(color) :
          QVariant(this->MissingColorIcon);

      case Qt::ToolTipRole:
      case Qt::StatusTipRole:
        return "Series Color";
        }
      }
    else if (idx.column() == LABEL)
      {
      QString label;
      if (this->Labels.contains(this->Visibilities[idx.row()].first))
        {
        label = this->Labels[this->Visibilities[idx.row()].first];
        }
      else
        {
        label = this->Visibilities[idx.row()].first;
        }

      switch (role)
        {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::StatusTipRole:
      case Qt::EditRole:
        return label;
        }
      }

    return QVariant();
    }

  virtual bool setData(const QModelIndex &idx, const QVariant &value,
    int role=Qt::EditRole)
    {
    if (idx.column() == VISIBILITY && role == Qt::CheckStateRole)
      {
      bool checkState = (value.toInt() == Qt::Checked);
      assert(idx.row() < this->Visibilities.size());
      this->Visibilities[idx.row()].second = checkState;
      emit this->dataChanged(idx, idx);
      return true;
      }
    else if (idx.column() == COLOR && role == Qt::EditRole)
      {
      assert(idx.row() < this->Visibilities.size());
      if (value.canConvert(QVariant::Color))
        {
        this->Colors[this->Visibilities[idx.row()].first] =
          value.value<QColor>();
        emit this->dataChanged(idx, idx);
        return true;
        }
      }
    else if (idx.column() == LABEL && role == Qt::EditRole)
      {
      assert(idx.row() < this->Visibilities.size());
      this->Labels[this->Visibilities[idx.row()].first] = value.toString();
      emit this->dataChanged(idx, idx);
      return true;
      }
    return false;
    }
  
  QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
    if (orientation == Qt::Horizontal &&
      (role == Qt::DisplayRole || role == Qt::ToolTipRole))
      {
      switch (section)
        {
      case VISIBILITY:
        return "Variable";
      case COLOR:
        return ""; 
      case LABEL:
        return "Legend Name";
        }
      }
    return this->Superclass::headerData(section, orientation, role);
    }

  QString seriesName(const QModelIndex& idx) const
    {
    if (idx.isValid() && idx.row() < this->Visibilities.size())
      {
      return this->Visibilities[idx.row()].first;
      }
    return QString();
    }

  void setVisibilities(const QVector<QPair<QString, bool> >& new_visibilies)
    {
    emit this->beginResetModel();
    this->Visibilities = new_visibilies;
    emit this->endResetModel();
    }

  const QVector<QPair<QString, bool> >& visibilities() const
    {
    return this->Visibilities;
    }

  void setLabels(const QVector<QPair<QString, QString> >& new_labels)
    {
    typedef QPair<QString, QString> item_type;
    foreach (const item_type& pair, new_labels)
      {
      this->Labels[pair.first] = pair.second;
      }

    if (this->rowCount() > 0)
      {
      emit this->dataChanged(this->index(0, LABEL),
        this->index(this->rowCount()-1, LABEL));
      }
    }

  const QVector<QPair<QString, QString> > labels() const
    {
    QVector<QPair<QString, QString> > reply;

    // return labels for the ones we have visibility information.
    typedef QPair<QString, bool> item_type;
    foreach (const item_type& pair, this->Visibilities)
      {
      if (this->Labels.contains(pair.first))
        {
        reply.push_back(QPair<QString, QString>(
            pair.first, this->Labels[pair.first]));
        }
      else
        {
        reply.push_back(QPair<QString, QString>(
            pair.first, pair.first));
        }
      }
    return reply;
    }

  void setColors(const QVector<QPair<QString, QColor> >& new_colors)
    {
    typedef QPair<QString, QColor> item_type;
    foreach (const item_type& pair, new_colors)
      {
      this->Colors[pair.first] = pair.second;
      }
    if (this->rowCount() > 0)
      {
      emit this->dataChanged(this->index(0, COLOR),
        this->index(this->rowCount()-1, COLOR));
      }
    }

  const QVector<QPair<QString, QColor> > colors() const
    {
    QVector<QPair<QString, QColor> > reply;

    // return labels for the ones we have visibility information.
    typedef QPair<QString, bool> item_type;
    foreach (const item_type& pair, this->Visibilities)
      {
      if (this->Colors.contains(pair.first))
        {
        reply.push_back(QPair<QString, QColor>(
            pair.first, this->Colors[pair.first]));
        }
      }
    return reply;
    }

  //--------- Drag-N-Drop support when enabled --------
  Qt::DropActions supportedDropActions() const
    {
    return this->SupportsReorder? (Qt::CopyAction | Qt::MoveAction) :
      this->Superclass::supportedDropActions();
    }

  QStringList mimeTypes() const
    {
    if (this->SupportsReorder)
      {
      QStringList types;
      types << "application/paraview.series.list";
      return types;
      }

    return this->Superclass::mimeTypes();
    }

  QMimeData *mimeData(const QModelIndexList &indexes) const
    {
    if (!this->SupportsReorder) { return this->Superclass::mimeData(indexes); }
    QMimeData *mime_data = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QList<QString> keys;
    foreach (const QModelIndex &idx, indexes)
      {
      QString name = this->seriesName(idx);
      if (!name.isEmpty() && !keys.contains(name))
        {
        keys << this->seriesName(idx);
        }
      }
    foreach (const QString& str, keys)
      {
      stream << str;
      }
    mime_data->setData("application/paraview.series.list", encodedData);
    return mime_data;
    }

  bool dropMimeData(const QMimeData *mime_data,
    Qt::DropAction action, int row, int column, const QModelIndex &parentIdx)
    {
    if (!this->SupportsReorder)
      {
      return this->Superclass::dropMimeData(mime_data,
        action, row, column, parentIdx);
      }
    if (action == Qt::IgnoreAction)
      {
      return true;
      }
    if (!mime_data->hasFormat("application/paraview.series.list"))
      {
      return false;
      }

    int beginRow = -1;
    if (row != -1)
      {
      beginRow = row;
      }
    else if (parentIdx.isValid())
      {
      beginRow = parentIdx.row();
      }
    else
      {
      beginRow = this->rowCount();
      }
    if (beginRow < 0)
      {
      return false;
      }

    QByteArray encodedData = mime_data->data("application/paraview.series.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    while (!stream.atEnd())
      {
      QString text;
      stream >> text;
      newItems << text;
      }

    // now re-order the visibilities list.
    QVector<QPair<QString, bool> > new_visibilies;
    QMap<QString, bool> to_insert;

    int real_begin_row = -1;
    for (int cc=0; cc< this->Visibilities.size(); cc++)
      {
      if (cc == beginRow)
        {
        real_begin_row = new_visibilies.size();
        }
      if (!newItems.contains(this->Visibilities[cc].first))
        {
        new_visibilies.push_back(this->Visibilities[cc]);
        }
      else
        {
        to_insert.insert(this->Visibilities[cc].first,
          this->Visibilities[cc].second);
        }
      }
    if (real_begin_row == -1)
      {
      real_begin_row = new_visibilies.size();
      }
    foreach (const QString& item, newItems)
      {
      if (to_insert.contains(item))
        {
        new_visibilies.insert(real_begin_row,
          QPair<QString, bool>(item, to_insert[item]));
        real_begin_row++;
        }
      }
    this->setVisibilities(new_visibilies);
    emit this->dataChanged(this->index(0, VISIBILITY),
      this->index(this->rowCount()-1, LABEL));
    return true;
    }

private:
  Q_DISABLE_COPY(pqSeriesParametersModel);
};

class pqSeriesEditorPropertyWidget::pqInternals
{
public:
  Ui::SeriesEditorPropertyWidget Ui;
  pqSeriesParametersModel Model;
  QMap<QString, int> Thickness;
  QMap<QString, int> Style;
  QMap<QString, int> MarkerStyle;
  QMap<QString, int> PlotCorner;
  bool RefreshingWidgets;

  pqInternals(bool supportsReorder, pqSeriesEditorPropertyWidget* self) 
    : Model(supportsReorder),
      RefreshingWidgets(false)
    {
    this->Ui.setupUi(self);
    this->Ui.wdgLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    this->Ui.wdgLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    this->Ui.wdgLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());

    this->Ui.SeriesTable->setDragEnabled(supportsReorder);
    this->Ui.SeriesTable->setDragDropMode(supportsReorder?
      QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    this->Ui.SeriesTable->horizontalHeader()->setHighlightSections(false);
    this->Ui.SeriesTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    this->Ui.SeriesTable->horizontalHeader()->setStretchLastSection(true);

    if (supportsReorder)
      {
      this->Ui.SeriesTable->setModel(&this->Model);
      }
    else
      {
      QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(self);
      proxyModel->setSourceModel(&this->Model);
      this->Ui.SeriesTable->setModel(proxyModel);
      this->Ui.SeriesTable->setSortingEnabled(true);
      }
    }

  //---------------------------------------------------------------------------
  // maps an index to that for this->Model. This is needed since we sometimes
  // use a proxy-model for sorting. When unsure, just call this. The logic here
  // handles correctly when this is called on an index already associated with
  // this->Model.
  QModelIndex modelIndex(const QModelIndex& idx) const
    {
    const QSortFilterProxyModel* proxyModel =
      qobject_cast<const QSortFilterProxyModel*>(idx.model());
    return proxyModel? proxyModel->mapToSource(idx) : idx;
    }
};

//-----------------------------------------------------------------------------
pqSeriesEditorPropertyWidget::pqSeriesEditorPropertyWidget(
  vtkSMProxy* smproxy, vtkSMPropertyGroup* smgroup, QWidget* parentObject)
  : Superclass(smproxy, parentObject),
  Internals(NULL)
{
  if (vtkSMProperty* smproperty = smgroup->GetProperty("SeriesVisibility"))
    {
    vtkPVXMLElement* hints = smproperty->GetHints()?
      smproperty->GetHints()->FindNestedElementByName("SeriesEditor") : NULL;
    int value = 0;
    bool supportsReorder = (hints != NULL &&
      hints->GetScalarAttribute("supports_reordering", &value) != 0 &&
      value == 1);
    this->Internals = new pqInternals(supportsReorder, this);
    }
  else
    {
    qCritical("SeriesVisibility property is required by pqSeriesEditorPropertyWidget."
      " This widget is not going to work.");
    return;
    }

  Ui::SeriesEditorPropertyWidget &ui = this->Internals->Ui;

  this->addPropertyLink(
    this, "seriesVisibility", SIGNAL(seriesVisibilityChanged()),
    smgroup->GetProperty("SeriesVisibility"));

  if (smgroup->GetProperty("SeriesLabel"))
    {
    this->addPropertyLink(
      this, "seriesLabel", SIGNAL(seriesLabelChanged()),
      smgroup->GetProperty("SeriesLabel"));
    }
  else
    {
    ui.SeriesTable->hideColumn(pqSeriesParametersModel::LABEL);
    }

  if (smgroup->GetProperty("SeriesColor"))
    {
    this->addPropertyLink(
      this, "seriesColor", SIGNAL(seriesColorChanged()),
      smgroup->GetProperty("SeriesColor"));
    }
  else
    {
    ui.SeriesTable->hideColumn(pqSeriesParametersModel::COLOR);
    }

  if (smgroup->GetProperty("SeriesLineThickness"))
    {
    this->addPropertyLink(
      this, "seriesLineThickness", SIGNAL(seriesLineThicknessChanged()),
      smgroup->GetProperty("SeriesLineThickness"));
    }
  else
    {
    ui.ThicknessLabel->hide();
    ui.Thickness->hide();
    }

  if (smgroup->GetProperty("SeriesLineStyle"))
    {
    this->addPropertyLink(
      this, "seriesLineStyle", SIGNAL(seriesLineStyleChanged()),
      smgroup->GetProperty("SeriesLineStyle"));
    }
  else
    {
    ui.StyleListLabel->hide();
    ui.StyleList->hide();
    }

  if (smgroup->GetProperty("SeriesMarkerStyle"))
    {
    this->addPropertyLink(
      this, "seriesMarkerStyle", SIGNAL(seriesMarkerStyleChanged()),
      smgroup->GetProperty("SeriesMarkerStyle"));
    }
  else
    {
    ui.MarkerStyleListLabel->hide();
    ui.MarkerStyleList->hide();
    }

  if (smgroup->GetProperty("SeriesPlotCorner"))
    {
    this->addPropertyLink(
      this, "seriesPlotCorner", SIGNAL(seriesPlotCornerChanged()),
      smgroup->GetProperty("SeriesPlotCorner"));
    }
  else
    {
    ui.AxisListLabel->hide();
    ui.AxisList->hide();
    }

  QObject::connect(&this->Internals->Model,
    SIGNAL(dataChanged(const QModelIndex &, const QModelIndex&)),
    this, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&)));

  this->connect(ui.SeriesTable, SIGNAL(doubleClicked(const QModelIndex&)),
    SLOT(onDoubleClicked(const QModelIndex&)));

  this->connect(
    ui.SeriesTable->selectionModel(),
    SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
    SLOT(refreshPropertiesWidgets()));

  this->connect(ui.Thickness, SIGNAL(valueChanged(int)),
    SLOT(savePropertiesWidgets()));
  this->connect(ui.StyleList,
    SIGNAL(currentIndexChanged(int)), SLOT(savePropertiesWidgets()));
  this->connect(ui.MarkerStyleList,
    SIGNAL(currentIndexChanged(int)), SLOT(savePropertiesWidgets()));
  this->connect(ui.AxisList,
    SIGNAL(currentIndexChanged(int)), SLOT(savePropertiesWidgets()));
}

//-----------------------------------------------------------------------------
pqSeriesEditorPropertyWidget::~pqSeriesEditorPropertyWidget()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::onDataChanged(
  const QModelIndex& topleft, const QModelIndex& btmright)
{
  // We don't need to worry about proxyModel here since we're directly observing
  // signal on this->Internals->Model.
  if (topleft.column() == 0)
    {
    emit this->seriesVisibilityChanged();
    }
  if (topleft.column() <= 1 &&
    btmright.column() >= 1)
    {
    emit this->seriesColorChanged();
    }
  if (btmright.column() >= 2)
    {
    emit this->seriesLabelChanged();
    }
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::onDoubleClicked(const QModelIndex& idx)
{
  // when user double-clicks on the color-swatch in the table view, we popup the
  // color selector dialog.
  if (idx.column() == 1)
    {
    QModelIndex index = this->Internals->modelIndex(idx);
    QColor color = this->Internals->Model.data(index,
      Qt::DecorationRole).value<QColor>();
    color = QColorDialog::getColor(color, this, "Choose Series Color",
      QColorDialog::DontUseNativeDialog);
    if (color.isValid())
      {
      this->Internals->Model.setData(index, color);
      }
    }
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesVisibility(
  const QList<QVariant> & values)
{
  QVector<QPair<QString, bool> > data;
  data.resize(values.size()/2);
  for (int cc=0; (cc + 1) < values.size(); cc+=2)
    {
    data[cc/2].first = values[cc].toString();
    data[cc/2].second = values[cc+1].toString() == "1";
    }
  this->Internals->Model.setVisibilities(data);
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesVisibility() const
{
  const QVector<QPair<QString, bool> > &data =
    this->Internals->Model.visibilities();

  QList<QVariant> reply;
  for (int cc=0; cc < data.size(); cc++)
    {
    reply.push_back(data[cc].first);
    reply.push_back(data[cc].second? "1" : "0");
    }
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesLabel(const QList<QVariant> & values)
{
  QVector<QPair<QString, QString> > data;
  data.resize(values.size()/2);
  for (int cc=0; (cc + 1) < values.size(); cc+=2)
    {
    data[cc/2].first = values[cc].toString();
    data[cc/2].second = values[cc+1].toString();
    }
  this->Internals->Model.setLabels(data);
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesLabel() const
{
  const QVector<QPair<QString, QString> > &data =
    this->Internals->Model.labels();
  QList<QVariant> reply;
  for (int cc=0; cc < data.size(); cc++)
    {
    reply.push_back(data[cc].first);
    reply.push_back(data[cc].second);
    }
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesColor(const QList<QVariant> & values)
{
  QVector<QPair<QString, QColor> > data;
  data.resize(values.size()/4);
  for (int cc=0; (cc + 3) < values.size(); cc+=4)
    {
    QColor color;
    color.setRedF(values[cc+1].toDouble());
    color.setGreenF(values[cc+2].toDouble());
    color.setBlueF(values[cc+3].toDouble());

    data[cc/4].first = values[cc].toString();
    data[cc/4].second = color;
    }
  this->Internals->Model.setColors(data);
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesColor() const
{
  const QVector<QPair<QString, QColor> > &data =
    this->Internals->Model.colors();
  QList<QVariant> reply;
  for (int cc=0; cc < data.size(); cc++)
    {
    reply.push_back(data[cc].first);
    reply.push_back(QString::number(data[cc].second.redF()));
    reply.push_back(QString::number(data[cc].second.greenF()));
    reply.push_back(QString::number(data[cc].second.blueF()));
    }
  return reply;
}

namespace
{
  template <class T>
  void setSeriesValues(QMap<QString, T>& data, const QList<QVariant>& values)
    {
    data.clear();
    for (int cc=0; (cc+1) < values.size(); cc+=2)
      {
      data[values[cc].toString()] = values[cc+1].value<T>();
      }
    }

  template <class T>
  void getSeriesValues(const QMap<QString, T>& data, QList<QVariant>& reply)
    {
    QMap<QString, int>::const_iterator iter = data.constBegin();
    for (;iter != data.constEnd(); ++iter)
      {
      reply.push_back(iter.key());
      reply.push_back(QString::number(iter.value()));
      }
    }
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesLineThickness(const QList<QVariant>& values)
{
  setSeriesValues<int>(this->Internals->Thickness, values);
  this->refreshPropertiesWidgets();
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesLineThickness() const
{
  QList<QVariant> reply;
  getSeriesValues(this->Internals->Thickness, reply);
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesLineStyle(const QList<QVariant>& values)
{
  setSeriesValues<int>(this->Internals->Style, values);
  this->refreshPropertiesWidgets();
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesLineStyle() const
{
  QList<QVariant> reply;
  getSeriesValues(this->Internals->Style, reply);
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesMarkerStyle(const QList<QVariant>& values)
{
  setSeriesValues<int>(this->Internals->MarkerStyle, values);
  this->refreshPropertiesWidgets();
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesMarkerStyle() const
{
  QList<QVariant> reply;
  getSeriesValues(this->Internals->MarkerStyle, reply);
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::setSeriesPlotCorner(const QList<QVariant>& values)
{
  setSeriesValues<int>(this->Internals->PlotCorner, values);
  this->refreshPropertiesWidgets();
}

//-----------------------------------------------------------------------------
QList<QVariant> pqSeriesEditorPropertyWidget::seriesPlotCorner() const
{
  QList<QVariant> reply;
  getSeriesValues(this->Internals->PlotCorner, reply);
  return reply;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::refreshPropertiesWidgets()
{
  Ui::SeriesEditorPropertyWidget &ui = this->Internals->Ui;
  pqSeriesParametersModel& model = this->Internals->Model;

  QModelIndex idx = this->Internals->modelIndex(ui.SeriesTable->currentIndex());
  QString key = this->Internals->Model.seriesName(idx);
  if (!idx.isValid() || key.isEmpty())
    {
    // nothing is selected, disable all properties widgets.
    ui.AxisList->setEnabled(false);
    ui.MarkerStyleList->setEnabled(false);
    ui.StyleList->setEnabled(false);
    ui.Thickness->setEnabled(false);
    return;
    }

  this->Internals->RefreshingWidgets = true;
  ui.Thickness->setValue(this->Internals->Thickness[key]);
  ui.Thickness->setEnabled(true);

  ui.StyleList->setCurrentIndex(this->Internals->Style[key]);
  ui.StyleList->setEnabled(true);

  ui.MarkerStyleList->setCurrentIndex(this->Internals->MarkerStyle[key]);
  ui.MarkerStyleList->setEnabled(true);

  ui.AxisList->setCurrentIndex(this->Internals->PlotCorner[key]);
  ui.AxisList->setEnabled(true);
  this->Internals->RefreshingWidgets = false;
}

//-----------------------------------------------------------------------------
void pqSeriesEditorPropertyWidget::savePropertiesWidgets()
{
  if (this->Internals->RefreshingWidgets) { return; }

  Ui::SeriesEditorPropertyWidget &ui = this->Internals->Ui;
  pqSeriesParametersModel& model = this->Internals->Model;

  QModelIndex idx = this->Internals->modelIndex(ui.SeriesTable->currentIndex());
  QString key = model.seriesName(idx);
  if (!idx.isValid() || key.isEmpty())
    {
    // nothing is selected
    return;
    }

  if (this->Internals->Thickness[key] != ui.Thickness->value())
    {
    this->Internals->Thickness[key] = ui.Thickness->value();
    emit this->seriesLineThicknessChanged();
    }

  if (this->Internals->Style[key] != ui.StyleList->currentIndex())
    {
    this->Internals->Style[key] = ui.StyleList->currentIndex();
    emit this->seriesLineStyleChanged();
    }

  if (this->Internals->MarkerStyle[key] != ui.MarkerStyleList->currentIndex())
    {
    this->Internals->MarkerStyle[key] = ui.MarkerStyleList->currentIndex();
    emit this->seriesMarkerStyleChanged();
    }

  if (this->Internals->PlotCorner[key] != ui.AxisList->currentIndex())
    {
    this->Internals->PlotCorner[key] = ui.AxisList->currentIndex();
    emit this->seriesPlotCornerChanged();
    }
}
