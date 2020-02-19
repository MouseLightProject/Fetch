#pragma once
#include <QtWidgets>
#include "devices\Microscope.h"
#include "devices\LinearScanMirror.h"
#include "devices\pockels.h"
#include "devices\Vibratome.h"
#include "common.h"
#include <google\protobuf\descriptor.h>

// TODO
// [ ] Update other views when there's a change
//     e.g. when there are multiple line edit controls for one parameter...a change
//          in one is communicated to all, but only one update to the Microscope
//          config is made.
//
// NOTES
//
// The idea here is that the main GUI class instances controllers for
// each parameter.  The controllers are then used as factories to create
// widgets used to build various interface elements.
//
// This controller should facilitate construction of common controls for
// the parameters, event connection, etc... so that the only thing a "user"
// widget needs to do is something like:
//
//   ui::PockelsController *p = app->pock();
//   layout->addWidget(p->createLineEdit();
//
// Events are setup by the controller so that different UI elements providing
// different views of a parameter may be synchronized.
//
// [?]  layers of getters and setters for device parameters
//      1. here (IGetSet)
//      2. Device level
//      3. configuration object level 
//      3. hardware  level
//      
// ADDING A PROPERTY
//      1. Declare your IGetSet class 
//      2. Define your Get_ and Set_ functions
//      3. Define your createValidator_

namespace fetch {
namespace ui {
  class MainWindow;

  class DevicePropControllerBase:public QObject
  {
    Q_OBJECT;
    protected:
      QLineEdit  *le_;      // do these really need to be here?
      QComboBox  *cmb_;
	  QCheckBox  *checkBox_; //DGA: checkBox_ is a pointer to a QCheckBox

    public:
      DevicePropControllerBase() : le_(NULL), cmb_(NULL), checkBox_(NULL) {} //DGA: Initiate the pointers le_, cmb_ and checkBox_ to NULL
      
    signals:
      void configUpdated();

    protected slots:
      virtual void report();
      virtual void updateLabel   (QWidget *source) =0;
      virtual void setByLineEdit (QWidget *source) =0;
      virtual void updateLineEdit(QWidget *source) =0;
      virtual void setByComboBox (QWidget *source) =0;
      virtual void updateComboBox(QWidget *source) =0;
      virtual void setByDoubleSpinBox   (QWidget *source) =0;
      virtual void updateByDoubleSpinBox(QWidget *source) =0;
	  virtual void setByCheckBox(QWidget *source) =0; //DGA: Virtual function setByCheckBox that takes in as argument source, a pointer to type QWidget; used to set the configuration by clicking the checkbox. "=0" means it is a pure virtual function: derived classes will have to override it, else they will be abstract (be a base class). These are slots
      virtual void updateCheckBox(QWidget *source) =0; //DGA: Virtual function updateCheckBox that takes in as argument source, a pointer to type QWidget; used to update the check box if changes occur in configuration ''

  };

  template<typename TDevice, typename TConfig, class TGetSetInterface> 
  class DevicePropController:public DevicePropControllerBase
  { 
    public:
      DevicePropController(TDevice *dc, char* label, MainWindow *parent);

      QLabel         *createLabel();
      QLineEdit      *createLineEdit();
      QDoubleSpinBox *createDoubleSpinBox();
      QComboBox      *createComboBox();
	  QCheckBox      *createCheckBox(); //DGA: Declaration of *createCheckBox function (*createCheckBox is a function, createCheckBox is a pointer to the function)
      QLabel         *createLabelAndAddToLayout(QFormLayout *layout);
      QLineEdit      *createLineEditAndAddToLayout(QFormLayout *layout);
      QDoubleSpinBox *createDoubleSpinBoxAndAddToLayout(QFormLayout *layout);
      QComboBox      *createComboBoxAndAddToLayout(QFormLayout *layout);
	  QCheckBox      *createCheckBoxAndAddToLayout(QFormLayout *layout); //DGA: Declaration of *createCheckBoxAndAddToLayout function, which takes in *layout which is of type QFormLayout

    protected:
      void updateLabel          (QWidget *source);
      void setByLineEdit        (QWidget *source);
      void updateLineEdit       (QWidget *source);
      void setByComboBox        (QWidget *source);
      void updateComboBox       (QWidget *source);
      void setByDoubleSpinBox   (QWidget *source);
      void updateByDoubleSpinBox(QWidget *source);
	  void setByCheckBox		(QWidget *source); //DGA: Overriding virtual function
      void updateCheckBox		(QWidget *source); //DGA: Overriding virtual function

    protected:
      TGetSetInterface interface_;

    private:
      TDevice    *dc_;
      QString     label_;

      QSignalMapper labelSignalMapper_;
      QSignalMapper lineEditSignalMapper_;
      QSignalMapper comboBoxSignalMapper_;
      QSignalMapper doubleSpinBoxSignalMapper_;
	  QSignalMapper checkBoxSignalMapper_; //DGA: checkBoxSignalMapper is of type QSignalMapper; collects parameterless signals and re-emits them with integer, string or widget parameters corresponding to the objec that sent the signal (doc.qt.io/qt-5/qsignalmapper.html)
      QSignalMapper configUpdateSignalMapper_;

  };

  template<typename TConfig> inline QString  ValueToQString(TConfig v);
  template<typename TConfig>        TConfig  QStringToValue(QString &s,bool *ok);
  template<typename TConfig>        TConfig  doubleToValue(double v);

  /* Note:
   * Don't really need a virtual base class here because the interface is
   * implemented via templates.  It's mostly just to document what's
   * required.
   */

  template<typename TDevice, typename TConfig>
  struct IGetSet
  { virtual void Set_(TDevice *dc, TConfig &v) = 0;
    virtual TConfig Get_(TDevice *dc)          = 0;
    virtual QValidator* createValidator_(QObject* parent) = 0;

    virtual const ::google::protobuf::EnumDescriptor* enum_descriptor(TDevice *dc) {return NULL;}
  };
  
#define	DECL_GETSET_CLASS(NAME,TDC,T) \
  struct NAME:public IGetSet<TDC,T> \
  { void Set_(TDC *dc, T &v);\
    T Get_(TDC *dc);\
    QValidator* createValidator_(QObject* parent);\
  }
#define	DECL_GETSET_DESC_CLASS(NAME,TDC,T) \
  struct NAME:public IGetSet<TDC,T> \
  { void Set_(TDC *dc, T &v);\
    T Get_(TDC *dc);\
    QValidator* createValidator_(QObject* parent);\
    const ::google::protobuf::EnumDescriptor* enum_descriptor(TDC *dc);\
  }

  // Video

  DECL_GETSET_CLASS(GetSetResonantTurn,device::Microscope,f64);
  DECL_GETSET_CLASS(GetSetLines,device::Microscope,i32);
  DECL_GETSET_CLASS(GetSetLSMVerticalRange,device::LinearScanMirror,f64);
  DECL_GETSET_CLASS(GetSetPockels,device::Pockels,u32);

  typedef DevicePropController<device::Microscope,f64,GetSetResonantTurn>           ResonantTurnController;
  typedef DevicePropController<device::Microscope, i32, GetSetLines>                  LinesController;
  typedef DevicePropController<device::LinearScanMirror,f64,GetSetLSMVerticalRange> LSMVerticalRangeController;
  typedef DevicePropController<device::Pockels,u32,GetSetPockels>                   PockelsController;

  //Pipeline
  DECL_GETSET_CLASS(GetSetFrameAverageCount, device::Microscope, unsigned int); //DGA: Added to get/set the frame average count
  typedef DevicePropController<device::Microscope, unsigned int, GetSetFrameAverageCount>      FrameAverageCountController; //DGA: Added to get/set the frame average count

  // Vibratome
  DECL_GETSET_CLASS(GetSetVibratomeAmplitude,device::Vibratome,u32);
  DECL_GETSET_CLASS(GetSetVibratomeFeedDist,device::Vibratome,double);
  DECL_GETSET_CLASS(GetSetVibratomeFeedVel ,device::Vibratome,double);
  DECL_GETSET_CLASS(GetSetVibratomeCutPosX ,device::Vibratome,double);
  DECL_GETSET_CLASS(GetSetVibratomeCutPosY ,device::Vibratome,double);
  DECL_GETSET_CLASS(GetSetVibratomeZOffset ,device::Vibratome,float);
  DECL_GETSET_CLASS(GetSetVibratomeThickness   ,device::Vibratome,float); //DGA: Renamed thick to thickness
  DECL_GETSET_DESC_CLASS(GetSetVibratomeFeedAxis,device::Vibratome,cfg::device::Vibratome::VibratomeFeedAxis);
  typedef DevicePropController<device::Vibratome,u32,GetSetVibratomeAmplitude>                                      VibratomeAmplitudeController;
  typedef DevicePropController<device::Vibratome,double,GetSetVibratomeFeedDist>                                    VibratomeFeedDisController;
  typedef DevicePropController<device::Vibratome,double,GetSetVibratomeFeedVel >                                    VibratomeFeedVelController;
  typedef DevicePropController<device::Vibratome,double,GetSetVibratomeCutPosX >                                    VibratomeFeedPosXController;
  typedef DevicePropController<device::Vibratome,double,GetSetVibratomeCutPosY >                                    VibratomeFeedPosYController;
  typedef DevicePropController<device::Vibratome,float ,GetSetVibratomeZOffset >                                    VibratomeZOffsetController;
  typedef DevicePropController<device::Vibratome,float ,GetSetVibratomeThickness >                                  VibratomeThicknessController; //DGA: Renamed thick to thickness
  typedef DevicePropController<device::Vibratome,cfg::device::Vibratome::VibratomeFeedAxis,GetSetVibratomeFeedAxis> VibratomeFeedAxisController;

  // Stack
  DECL_GETSET_CLASS(GetSetZPiezoMin ,device::ZPiezo,f64);
  DECL_GETSET_CLASS(GetSetZPiezoMax ,device::ZPiezo,f64);
  DECL_GETSET_CLASS(GetSetZPiezoStep,device::ZPiezo,f64);
  typedef DevicePropController<device::ZPiezo,f64,GetSetZPiezoMin>  ZPiezoMinController;
  typedef DevicePropController<device::ZPiezo,f64,GetSetZPiezoMax>  ZPiezoMaxController;
  typedef DevicePropController<device::ZPiezo,f64,GetSetZPiezoStep> ZPiezoStepController;

  // Stage
  DECL_GETSET_CLASS(GetSetStagePosX,device::Stage,float);
  DECL_GETSET_CLASS(GetSetStagePosY,device::Stage,float);
  DECL_GETSET_CLASS(GetSetStagePosZ,device::Stage,float);
  DECL_GETSET_CLASS(GetSetStageVelX,device::Stage,float);
  DECL_GETSET_CLASS(GetSetStageVelY,device::Stage,float);
  DECL_GETSET_CLASS(GetSetStageVelZ,device::Stage,float);
  typedef DevicePropController<device::Stage,float,GetSetStagePosX> StagePosXController;
  typedef DevicePropController<device::Stage,float,GetSetStagePosY> StagePosYController;
  typedef DevicePropController<device::Stage,float,GetSetStagePosZ> StagePosZController;
  typedef DevicePropController<device::Stage,float,GetSetStageVelX> StageVelXController;
  typedef DevicePropController<device::Stage,float,GetSetStageVelY> StageVelYController;
  typedef DevicePropController<device::Stage,float,GetSetStageVelZ> StageVelZController;

  // FOV
  DECL_GETSET_CLASS(GetSetOverlapZ,device::FieldOfViewGeometry,float);
  typedef DevicePropController<device::FieldOfViewGeometry,float,GetSetOverlapZ> FOVOverlapZController;
  
  //DGA: Surface Find
  DECL_GETSET_CLASS(GetSetSurfaceFindIntesityThreshold, device::Microscope, float);
  typedef DevicePropController<device::Microscope, float, GetSetSurfaceFindIntesityThreshold> SurfaceFindIntensityThresholdController;

  // AutoTile
  DECL_GETSET_CLASS(GetSetAutoTileZOff             ,device::Microscope,float);
  DECL_GETSET_CLASS(GetSetAutoTileZMax             ,device::Microscope,float);
  DECL_GETSET_CLASS(GetSetAutoTileTimeoutMs        ,device::Microscope,unsigned);
  DECL_GETSET_CLASS(GetSetAutoTileChan             ,device::Microscope,unsigned);
  DECL_GETSET_CLASS(GetSetAutoTileIntesityThreshold,device::Microscope,float);
  DECL_GETSET_CLASS(GetSetAutoTileAreaThreshold    ,device::Microscope,float);
  DECL_GETSET_CLASS(GetSetAutoTileScheduleStopAfterNthCut, device::Microscope, bool);
  DECL_GETSET_CLASS(GetSetAutoTileNthCutToStopAfter, device::Microscope, unsigned);
  typedef DevicePropController<device::Microscope,float   ,GetSetAutoTileZOff>              AutoTileZOffController;
  typedef DevicePropController<device::Microscope,float   ,GetSetAutoTileZMax>              AutoTileZMaxController;
  typedef DevicePropController<device::Microscope,unsigned,GetSetAutoTileTimeoutMs>         AutoTileTimeoutMsController;
  typedef DevicePropController<device::Microscope,unsigned,GetSetAutoTileChan>              AutoTileChanController;
  typedef DevicePropController<device::Microscope,float   ,GetSetAutoTileIntesityThreshold> AutoTileIntensityThresholdController;
  typedef DevicePropController<device::Microscope,float   ,GetSetAutoTileAreaThreshold>     AutoTileAreaThresholdController;
  typedef DevicePropController<device::Microscope,bool   ,GetSetAutoTileScheduleStopAfterNthCut>  AutoTileScheduleStopAfterNthCutController;
  typedef DevicePropController<device::Microscope,unsigned, GetSetAutoTileNthCutToStopAfter>  AutoTileNthCutToStopAfterController;
}} //end fetch::ui

  ////////////////////////////////////////////////////////////////////////////
  //
  // Implementation
  //
  ////////////////////////////////////////////////////////////////////////////

namespace fetch {
namespace ui {
  
  // The idea here is:
  // When a widget gets changed:
  //    1. the signal from the widget is connected to a signal mapper
  //    2. the mapper triggers another signal that calls an update-from-widget function.
  //    -  Right now, line edits are handled specifically, but one could use type information
  //       to get the data on a case by case basis.
  //    -  The "GetSetInterface" defines how the value actually gets written to the config.
  // When the config is changed, the MainWindow should be notified via the configUpdate() signal.
  //    1. That signal is connected on construction to the property controller's configUpdate().
  //    2. This is mapped to an "update" function that updates viewing widgets.
  //  
  // It Should be easy to add support for multiple widgets of differnt kinds, but so far I'm just
  // using this for the ubiquitous LineEdit.

  // Protobuf types that are giving me trouble
  //  - enums    - currently using a custom combo box
  //  - repeated - currently using a custom table
  template<typename TDevice, typename TConfig, class TGetSetInterface> 
  DevicePropController<TDevice,TConfig,TGetSetInterface>::
    DevicePropController(TDevice *dc, char* label, MainWindow *parent)
  : dc_(dc)
  , label_(label)
  { 
    connect(&configUpdateSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(updateLabel(QWidget*)));

    connect(&lineEditSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(setByLineEdit(QWidget*)));
    connect(&configUpdateSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(updateLineEdit(QWidget*)));

    connect(&comboBoxSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(setByComboBox(QWidget*)));
    connect(&configUpdateSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(updateComboBox(QWidget*)));

    connect(&doubleSpinBoxSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(setByDoubleSpinBox(QWidget*)));
    connect(&configUpdateSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(updateByDoubleSpinBox(QWidget*)));    

	connect(&checkBoxSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(setByCheckBox(QWidget*))); //DGA: The pointer (address) to each QCheckBox has been mapped to each checkbox, so when map() is called on checkBoxSignalMapper_, mapped(checkBox address) will be emitted, which will in turn call the setByCheckBox slot of the appropriate checkBox. "this" being the pointer to the DevicePropController object.
    connect(&configUpdateSignalMapper_,SIGNAL(mapped(QWidget*)),this,SLOT(updateCheckBox(QWidget*))); //DGA: The pointer (address) to each QCheckBox has been mapped to the object of the class DevicePropController for which it was created, so when map() is called on configUpdateSignalMapper_, mapped(checkBox address) will be emitted, which will in turn call the updateCheckBox slot of the appropriate checkBox

    connect(parent,SIGNAL(configUpdated()),this,SIGNAL(configUpdated())); //DGA: When the parent signals configUpdated(), "this" object also signals configUpdated()
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    updateLabel(QWidget* source)
  {
    QLabel* l = qobject_cast<QLabel*>(source);
    if(l)
    { QString s = ValueToQString(interface_.Get_(dc_));
      l->setText(s);
    }
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    setByLineEdit(QWidget* source)
  { bool ok;  
    QLineEdit* le = qobject_cast<QLineEdit*>(source);
    Guarded_Assert(le);

    TConfig v = QStringToValue<TConfig>(le->text(),&ok);
    if(ok)
      interface_.Set_(dc_,v);
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    updateLineEdit(QWidget* source)
  {
    QLineEdit* le = qobject_cast<QLineEdit*>(source);
    if(le)
    { QString s = ValueToQString(interface_.Get_(dc_));
      le_->setText(s);
    }
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    setByComboBox(QWidget* source)
  {   
    QComboBox* w = qobject_cast<QComboBox*>(source);
    if(!w) return;

    int v = w->currentIndex();
    const ::google::protobuf::EnumDescriptor* d =  interface_.enum_descriptor(dc_);
    TConfig vv = (TConfig) (d->value(v)->number());
    interface_.Set_(dc_,vv);
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    updateComboBox(QWidget* source)
  {
    QComboBox* w = qobject_cast<QComboBox*>(source);
    if(w)
    { 
      int i,v = interface_.Get_(dc_);
      const ::google::protobuf::EnumDescriptor* d =  interface_.enum_descriptor(dc_);
      for(i=0;i<d->value_count();++i) // search for value in combo box; <i> will correspond by construction
        if(d->value(i)->number()==v)
          break;
      if(i==d->value_count())
        error("%s(%d) Did not recognize value.",__FILE__,__LINE__); // getting here is probably a logic error in the code
      w->setCurrentIndex(i);
    }
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    setByDoubleSpinBox(QWidget* source)
  {   
    QDoubleSpinBox* w = qobject_cast<QDoubleSpinBox*>(source);
    if(!w) return;
    TConfig vv = doubleToValue<TConfig> (w->value());
    interface_.Set_(dc_,vv);
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    updateByDoubleSpinBox(QWidget* source)
  {
    QDoubleSpinBox* w = qobject_cast<QDoubleSpinBox*>(source);
    if(w)
    { double v = interface_.Get_(dc_);
      w->blockSignals(true);
      w->setValue(v);
      w->blockSignals(false);
    }
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    setByCheckBox(QWidget* source)
  { QCheckBox* checkBox = qobject_cast<QCheckBox*>(source); //DGA: Get the pointer to the QCheckBox, returns 0 if source is not of type QCheckBox*
	if(checkBox) //DGA: If source was a checkBox pointer
	{ TConfig isChecked = doubleToValue<TConfig> (checkBox->isChecked()); //DGA: isChecked is true/flase if checkBox is checked/unchecked
      interface_.Set_(dc_, isChecked); //DGA: Calls the setter of the interface_ (GetSet class object) for the device pointed to by dc_ and using isChecked as the set value
	}
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  void DevicePropController<TDevice,TConfig,TGetSetInterface>::
    updateCheckBox(QWidget* source)
  {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(source); //DGA: Get the pointer to the QCheckBox, returns 0 if source is not of type QCheckBox*
    if(checkBox) //DGA: If source was a checkBox pointer
	{ bool isChecked = interface_.Get_(dc_); //DGA: Calls the getter of the interface (GetSetClass object), which in this case will return whether or not the checkbox should be checked
      checkBox->setChecked(isChecked); //DGA: Checks/unchecks the checkbox as determined by isChecked
	}
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QLabel* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createLabel()
  { TConfig vv = interface_.Get_(dc_);
    QString s = ValueToQString(vv);
    QLabel *l = new QLabel(s);

    //labelSignalMapper_.setMapping(l,l);
    configUpdateSignalMapper_.setMapping(this,l);
    connect(this,SIGNAL(configUpdated()),&configUpdateSignalMapper_,SLOT(map()));

    return l;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QLineEdit* DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createLineEdit()
  { TConfig vv = interface_.Get_(dc_);
    QString s = ValueToQString(vv);
    le_ = new QLineEdit(s);    
    QValidator *v = interface_.createValidator_(le_);
    if(v)
      le_->setValidator(v);

    QStringListModel *history = new QStringListModel(this);
    QCompleter *c = new QCompleter(history,this);
    c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    c->setModelSorting(QCompleter::UnsortedModel);
    le_->setCompleter(c);

    lineEditSignalMapper_.setMapping(le_,le_);
    connect(le_,SIGNAL(editingFinished()),&lineEditSignalMapper_,SLOT(map()));
    connect(le_,SIGNAL(editingFinished()),this,SLOT(report()));

    configUpdateSignalMapper_.setMapping(this,le_);
    connect(this,SIGNAL(configUpdated()),&configUpdateSignalMapper_,SLOT(map()));

    return le_;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QDoubleSpinBox* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createDoubleSpinBox()
  { TConfig vv = interface_.Get_(dc_);
    QString s = ValueToQString(vv);
    //QValidator *v = interface_.createValidator_(le_);
    QDoubleSpinBox *w = new QDoubleSpinBox();
    w->setKeyboardTracking(false);
    w->setValue(vv);
    //w->lineEdit()->setValidator(v);

    doubleSpinBoxSignalMapper_.setMapping(w,w);
    connect(w,SIGNAL(valueChanged(double)),&doubleSpinBoxSignalMapper_,SLOT(map()));
    //connect(w,SIGNAL(valueChanged(double d)),this,SLOT(report()));

    configUpdateSignalMapper_.setMapping(this,w);    
    connect(this,SIGNAL(configUpdated()),&configUpdateSignalMapper_,SLOT(map()));    
    return w;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QComboBox* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createComboBox()
  { TConfig vv = interface_.Get_(dc_);
    cmb_ = new QComboBox;

    const ::google::protobuf::EnumDescriptor* d =  interface_.enum_descriptor(dc_);
    for(int i=0;i<d->value_count();++i) // populate combo box
      cmb_->addItem(d->value(i)->name().c_str(), d->value(i)->number());
    cmb_->setEditable(false);      
    updateComboBox(cmb_);
    
    comboBoxSignalMapper_.setMapping(cmb_,cmb_);
    connect(cmb_,SIGNAL(currentIndexChanged(int)),&comboBoxSignalMapper_,SLOT(map()));
    //connect(cmb_,SIGNAL(currentIndexChanged(int)),this,SLOT(report()));

    configUpdateSignalMapper_.setMapping(this,cmb_);
    connect(this,SIGNAL(configUpdated()),&configUpdateSignalMapper_,SLOT(map()));   
    return cmb_;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QCheckBox* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createCheckBox() //DGA: Function to create the checkBox
  { checkBox_ = new QCheckBox; //DGA: checkBox_ now points to a dynamically allocated region of memory for a QCheckBox
    updateCheckBox(checkBox_); //DGA: updateCheckBox is called, which will initialize the checkBox's checked status to equal the value set in the config
    
    checkBoxSignalMapper_.setMapping(checkBox_,checkBox_); //DGA: setMapping of checkBoxSignalMapper is called, mapping the checkBox_'s address to the checkBox_
    connect(checkBox_,SIGNAL(clicked()),&checkBoxSignalMapper_,SLOT(map())); //DGA: clicking the checkbox causes the slot map() of checkBoxSignalMapper to execute, which emits the mapped(checkBox_ address) signal. This will in turn call slot setByCheckBox of the appropriate checkbox.

    configUpdateSignalMapper_.setMapping(this,checkBox_); //DGA: setMapping of configUpdateSignalMapper is called, mapping the checkBox_'s address to this DevicePropController
    connect(this,SIGNAL(configUpdated()),&configUpdateSignalMapper_,SLOT(map())); //DGA: When configUpdated() of this DevicePropController is called, the slot map() of configUpdatedSignalMapper_ is called, which emits the mapped(checkBox_ address) signal. This will in turn call the slot updateCheckBox of the appropriate checkbox. 
    return checkBox_; //DGA: The checkBox_ pointer is returned
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QLabel* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createLabelAndAddToLayout( QFormLayout *layout )
  { QLabel *l = createLabel();
    layout->addRow(label_,l);
    return l;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QLineEdit* DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createLineEditAndAddToLayout( QFormLayout *layout )
  { QLineEdit *le = createLineEdit();
    layout->addRow(label_,le);
    return le;
  }
  
  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QDoubleSpinBox* 
    DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createDoubleSpinBoxAndAddToLayout( QFormLayout *layout )
  { QDoubleSpinBox *w = createDoubleSpinBox();
    layout->addRow(label_,w);
    return w;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QComboBox* DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createComboBoxAndAddToLayout( QFormLayout *layout )
  { QComboBox *w = createComboBox();
    layout->addRow(label_,w);
    return w;
  }

  template<typename TDevice, typename TConfig, class TGetSetInterface>
  QCheckBox* DevicePropController<TDevice,TConfig,TGetSetInterface>::
    createCheckBoxAndAddToLayout( QFormLayout *layout ) //DGA: Create checkbox and add to layout
  { QCheckBox *checkBox = createCheckBox(); //DGA: Creates the checkBox with all the necessary mappings
	checkBox->setText(label_); //DGA: Sets the text of checkbox to label_, making the text on the right side. label_ is just the provided label since createLabel() was never called.
    layout->addRow("",checkBox);//DGA: Adds the row -- "" checkBox label_ -- to the layout. Since label is set to "", the checkbox is aligned properly with the other fields in the widget
    return checkBox; //DGA: Returns the checkBox pointer
  }  
}} //end fetch::ui
