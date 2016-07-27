#include "ModemProperties.h"
#include "CubicSDR.h"

ModemProperties::ModemProperties(wxWindow *parent, wxWindowID winid,
         const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxPanel(parent, winid, pos, size, style, name) {
    
   	m_propertyGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_DEFAULT_STYLE | wxPG_NO_INTERNAL_BORDER);

    bSizer = new wxBoxSizer( wxVERTICAL );
    
    bSizer->Add(m_propertyGrid, 1, wxEXPAND, 0);
    
    this->SetSizer(bSizer);
    
    m_propertyGrid->Connect( wxEVT_PG_ITEM_COLLAPSED, wxPropertyGridEventHandler( ModemProperties::OnCollapse ), NULL, this );
    m_propertyGrid->Connect( wxEVT_PG_ITEM_EXPANDED, wxPropertyGridEventHandler( ModemProperties::OnExpand ), NULL, this );
    m_propertyGrid->Connect( wxEVT_PG_CHANGED, wxPropertyGridEventHandler( ModemProperties::OnChange ), NULL, this );
    this->Connect( wxEVT_SHOW, wxShowEventHandler( ModemProperties::OnShow ), NULL, this );
    
    this->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( ModemProperties::OnMouseEnter ), NULL, this);
    this->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( ModemProperties::OnMouseLeave ), NULL, this);

    updateTheme();

    mouseInView = false;
    collapsed = false;
}

void ModemProperties::OnShow(wxShowEvent & /* event */) {
    updateTheme();
}

void ModemProperties::updateTheme() {
    wxColour bgColor(
                 (unsigned char) (ThemeMgr::mgr.currentTheme->generalBackground.r * 255.0),
                 (unsigned char) (ThemeMgr::mgr.currentTheme->generalBackground.g * 255.0),
                 (unsigned char) (ThemeMgr::mgr.currentTheme->generalBackground.b * 255.0));

    wxColour textColor(
                       (unsigned char) (ThemeMgr::mgr.currentTheme->text.r * 255.0),
                       (unsigned char) (ThemeMgr::mgr.currentTheme->text.g * 255.0),
                       (unsigned char) (ThemeMgr::mgr.currentTheme->text.b * 255.0));
    
    wxColour btn(
                       (unsigned char) (ThemeMgr::mgr.currentTheme->button.r * 255.0),
                       (unsigned char) (ThemeMgr::mgr.currentTheme->button.g * 255.0),
                       (unsigned char) (ThemeMgr::mgr.currentTheme->button.b * 255.0));

    wxColour btnHl(
                 (unsigned char) (ThemeMgr::mgr.currentTheme->buttonHighlight.r * 255.0),
                 (unsigned char) (ThemeMgr::mgr.currentTheme->buttonHighlight.g * 255.0),
                 (unsigned char) (ThemeMgr::mgr.currentTheme->buttonHighlight.b * 255.0));


    m_propertyGrid->SetEmptySpaceColour(bgColor);
    m_propertyGrid->SetCellBackgroundColour(bgColor);
    m_propertyGrid->SetCellTextColour(textColor);
    m_propertyGrid->SetSelectionTextColour(bgColor);
    m_propertyGrid->SetSelectionBackgroundColour(btnHl);
    m_propertyGrid->SetCaptionTextColour(bgColor);
    m_propertyGrid->SetCaptionBackgroundColour(btn);
    m_propertyGrid->SetLineColour(btn);
}

ModemProperties::~ModemProperties() {
    
}

void ModemProperties::initProperties(ModemArgInfoList newArgs) {
    args = newArgs;

    bSizer->Layout();
    m_propertyGrid->Clear();

    if (newArgs.size() == 0) {
        Hide();
        return;
    } else {
        Show();
    }
    
    m_propertyGrid->Append(new wxPropertyCategory("Modem Settings"));
    
    ModemArgInfoList::const_iterator args_i;
    
    for (args_i = args.begin(); args_i != args.end(); args_i++) {
        ModemArgInfo arg = (*args_i);
        props[arg.key] = addArgInfoProperty(m_propertyGrid, arg);
    }
    
    m_propertyGrid->FitColumns();
    
    if (collapsed) {
        m_propertyGrid->CollapseAll();
    }
}

wxPGProperty *ModemProperties::addArgInfoProperty(wxPropertyGrid *pg, ModemArgInfo arg) {
    wxPGProperty *prop = nullptr;
    
    int intVal;
    double floatVal;
    std::vector<std::string>::iterator stringIter;
    
    switch (arg.type) {
        case ModemArgInfo::INT:
            try {
                intVal = std::stoi(arg.value);
            } catch (std::invalid_argument e) {
                intVal = 0;
            }
            prop = pg->Append( new wxIntProperty(arg.name, wxPG_LABEL, intVal) );
            if (arg.range.minimum() != arg.range.maximum()) {
                pg->SetPropertyAttribute( prop, wxPG_ATTR_MIN, arg.range.minimum());
                pg->SetPropertyAttribute( prop, wxPG_ATTR_MAX, arg.range.maximum());
            }
            break;
        case ModemArgInfo::FLOAT:
            try {
                floatVal = std::stod(arg.value);
            } catch (std::invalid_argument e) {
                floatVal = 0;
            }
            prop = pg->Append( new wxFloatProperty(arg.name, wxPG_LABEL, floatVal) );
            if (arg.range.minimum() != arg.range.maximum()) {
                pg->SetPropertyAttribute( prop, wxPG_ATTR_MIN, arg.range.minimum());
                pg->SetPropertyAttribute( prop, wxPG_ATTR_MAX, arg.range.maximum());
            }
            break;
        case ModemArgInfo::BOOL:
            prop = pg->Append( new wxBoolProperty(arg.name, wxPG_LABEL, (arg.value=="true")) );
            break;
        case ModemArgInfo::STRING:
            if (arg.options.size()) {
                intVal = 0;
                prop = pg->Append( new wxEnumProperty(arg.name, wxPG_LABEL) );
                for (stringIter = arg.options.begin(); stringIter != arg.options.end(); stringIter++) {
                    std::string optName = (*stringIter);
                    std::string displayName = optName;
                    if (arg.optionNames.size()) {
                        displayName = arg.optionNames[intVal];
                    }
                    
                    prop->AddChoice(displayName);
                    if ((*stringIter)==arg.value) {
                        prop->SetChoiceSelection(intVal);
                    }
                    
                    intVal++;
                }
            } else {
                prop = pg->Append( new wxStringProperty(arg.name, wxPG_LABEL, arg.value) );
            }
            break;
        case ModemArgInfo::PATH_DIR:
            break;
        case ModemArgInfo::PATH_FILE:
            break;
        case ModemArgInfo::COLOR:
            break;
    }
    
    if (prop != NULL) {
        prop->SetHelpString(arg.key + ": " + arg.description);
    }
    
    return prop;
}

std::string ModemProperties::readProperty(std::string key) {
    int i = 0;
    ModemArgInfoList::const_iterator args_i;

    for (args_i = args.begin(); args_i != args.end(); args_i++) {
        ModemArgInfo arg = (*args_i);
        if (arg.key == key) {
            wxPGProperty *prop = props[key];
            
            std::string result = "";
            if (arg.type == ModemArgInfo::STRING && arg.options.size()) {
                return arg.options[prop->GetChoiceSelection()];
            } else if (arg.type == ModemArgInfo::BOOL) {
                return (prop->GetValueAsString()=="True")?"true":"false";
            } else {
                return prop->GetValueAsString().ToStdString();
            }
        }
        i++;
    }
    return "";
}

void ModemProperties::OnChange(wxPropertyGridEvent &event) {
    DemodulatorInstance *inst = wxGetApp().getDemodMgr().getLastActiveDemodulator();
    
    if (!inst) {
        return;
    }
    
    std::map<std::string, wxPGProperty *>::const_iterator prop_i;
    for (prop_i = props.begin(); prop_i != props.end(); prop_i++) {
        if (prop_i->second == event.m_property) {
            std::string key = prop_i->first;
            std::string value = readProperty(prop_i->first);
            inst->writeModemSetting(key, value);
            return;
        }
    }
}

void ModemProperties::OnCollapse(wxPropertyGridEvent &event) {
    collapsed = true;
}

void ModemProperties::OnExpand(wxPropertyGridEvent &event) {
    collapsed = false;
}

void ModemProperties::OnMouseEnter(wxMouseEvent & /* event */) {
    mouseInView = true;
}

void ModemProperties::OnMouseLeave(wxMouseEvent & /* event */) {
    mouseInView = false;
}

bool ModemProperties::isMouseInView() {
    return mouseInView || (m_propertyGrid && m_propertyGrid->IsEditorFocused());
}

bool ModemProperties::isCollapsed() {
    return collapsed;
}

void ModemProperties::fitColumns() {
    m_propertyGrid->FitColumns();
}
