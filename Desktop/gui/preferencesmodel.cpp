#include "preferencesmodel.h"
#include "utils.h"
#include "utilities/qutils.h"

#include "utilities/settings.h"
#include "gui/messageforwarder.h"
#include "qquick/jasptheme.h"
#include "utilities/languagemodel.h"
#include <QFontDatabase>
#include "modules/ribbonmodel.h"
#include "utilities/qutils.h"
#include "utilities/appdirs.h"
#include "enginedefinitions.h"

using namespace std;

PreferencesModel * PreferencesModel::_singleton = nullptr;

PreferencesModel::PreferencesModel(QObject *parent) :
	QObject(parent)
{
	if(_singleton) throw std::runtime_error("PreferencesModel can only be instantiated once!");
	_singleton = this;
	
	connect(this,					&PreferencesModel::missingValuesChanged,		this, &PreferencesModel::updateUtilsMissingValues		);

	connect(this,					&PreferencesModel::useDefaultPPIChanged,		this, &PreferencesModel::onUseDefaultPPIChanged			);
	connect(this,					&PreferencesModel::defaultPPIChanged,			this, &PreferencesModel::onDefaultPPIChanged			);
	connect(this,					&PreferencesModel::customPPIChanged,			this, &PreferencesModel::onCustomPPIChanged				);
	connect(this,					&PreferencesModel::useDefaultPPIChanged,		this, &PreferencesModel::plotPPIPropChanged				);
	connect(this,					&PreferencesModel::defaultPPIChanged,			this, &PreferencesModel::plotPPIPropChanged				);
	connect(this,					&PreferencesModel::customPPIChanged,			this, &PreferencesModel::plotPPIPropChanged				);
	connect(this,					&PreferencesModel::plotBackgroundChanged,		this, &PreferencesModel::whiteBackgroundChanged			);
	connect(this,					&PreferencesModel::modulesRememberChanged,		this, &PreferencesModel::resetRememberedModules			);

	connect(this,					&PreferencesModel::jaspThemeChanged,			this, &PreferencesModel::setCurrentThemeNameFromClass,	Qt::QueuedConnection);
	connect(this,					&PreferencesModel::currentThemeNameChanged,		this, &PreferencesModel::onCurrentThemeNameChanged		);

	connect(this,					&PreferencesModel::safeGraphicsChanged,			this, &PreferencesModel::animationsOnChanged			); // So animationsOn *might* not be changed, but it  doesnt matter
	connect(this,					&PreferencesModel::disableAnimationsChanged,	this, &PreferencesModel::animationsOnChanged			);

	connect(LanguageModel::lang(),	&LanguageModel::currentLanguageChanged,			this, &PreferencesModel::languageCodeChanged			);

	_loadDatabaseFont();
}

void PreferencesModel::browseSpreadsheetEditor()
{
	
	QString filter = "File Description (*.*)";
	QString applicationfolder;

#ifdef _WIN32
	applicationfolder = "c:\\Program Files";
#elif __APPLE__
	applicationfolder = "/Applications";
#else
	applicationfolder = "/usr/bin";
#endif

	QString filename = MessageForwarder::browseOpenFile(tr("Select a file..."), applicationfolder, filter);

	if (filename != "")
		setCustomEditor(filename);
	
}

void PreferencesModel::browseDeveloperFolder()
{
	QString defaultfolder = developerFolder();
	if (defaultfolder.isEmpty())
	{
#ifdef _WIN32
		defaultfolder = "c:\\";
#else
		defaultfolder = "~";
#endif
	}

	QString folder = MessageForwarder::browseOpenFolder(tr("Select a folder..."), defaultfolder);

	if (!folder.isEmpty())
		setDeveloperFolder(folder);
		
}

#define GET_PREF_FUNC(TYPE, NAME, SETTING, TO_FUNC)	TYPE PreferencesModel::NAME() const { return Settings::value(SETTING).TO_FUNC; }
#define GET_PREF_FUNC_BOOL(NAME, SETTING)					GET_PREF_FUNC(bool,		NAME, SETTING, toBool())
#define GET_PREF_FUNC_INT(NAME, SETTING)					GET_PREF_FUNC(int,		NAME, SETTING, toInt())
#define GET_PREF_FUNC_STR(NAME, SETTING)					GET_PREF_FUNC(QString,	NAME, SETTING, toString())
#define GET_PREF_FUNC_DBL(NAME, SETTING)					GET_PREF_FUNC(double,	NAME, SETTING, toDouble())
#define GET_PREF_FUNC_WHT(NAME, SETTING)					GET_PREF_FUNC(bool,		NAME, SETTING, toString() == "white")


GET_PREF_FUNC_BOOL(	fixedDecimals,				Settings::FIXED_DECIMALS							)
GET_PREF_FUNC_INT(	numDecimals,				Settings::NUM_DECIMALS								)
GET_PREF_FUNC_BOOL(	exactPValues,				Settings::EXACT_PVALUES								)
GET_PREF_FUNC_BOOL(	normalizedNotation,			Settings::NORMALIZED_NOTATION						)
GET_PREF_FUNC_BOOL(	dataAutoSynchronization,	Settings::DATA_AUTO_SYNCHRONIZATION					)
GET_PREF_FUNC_BOOL(	useDefaultEditor,			Settings::USE_DEFAULT_SPREADSHEET_EDITOR			)
GET_PREF_FUNC_STR(	customEditor,				Settings::SPREADSHEET_EDITOR_NAME					)
GET_PREF_FUNC_STR(	developerFolder,			Settings::DEVELOPER_FOLDER							)
GET_PREF_FUNC_BOOL(	useDefaultPPI,				Settings::PPI_USE_DEFAULT							)
GET_PREF_FUNC_INT(	customPPI,					Settings::PPI_CUSTOM_VALUE							)
GET_PREF_FUNC_WHT(	whiteBackground,			Settings::IMAGE_BACKGROUND							)
GET_PREF_FUNC_STR(	plotBackground,				Settings::IMAGE_BACKGROUND							)
GET_PREF_FUNC_BOOL(	developerMode,				Settings::DEVELOPER_MODE							)
GET_PREF_FUNC_BOOL(	customThresholdScale,		Settings::USE_CUSTOM_THRESHOLD_SCALE				)
GET_PREF_FUNC_INT(	thresholdScale,				Settings::THRESHOLD_SCALE							)
GET_PREF_FUNC_BOOL(	logToFile,					Settings::LOG_TO_FILE								)
GET_PREF_FUNC_INT(	logFilesMax,				Settings::LOG_FILES_MAX								)
GET_PREF_FUNC_INT(	maxFlickVelocity,			Settings::QML_MAX_FLICK_VELOCITY					)
GET_PREF_FUNC_BOOL(	modulesRemember,			Settings::MODULES_REMEMBER							)
GET_PREF_FUNC_BOOL(	safeGraphics,				Settings::SAFE_GRAPHICS_MODE						)
GET_PREF_FUNC_STR(	cranRepoURL,				Settings::CRAN_REPO_URL								)
GET_PREF_FUNC_BOOL(	githubPatUseDefault,		Settings::GITHUB_PAT_USE_DEFAULT					)
GET_PREF_FUNC_STR(	currentThemeName,			Settings::THEME_NAME								)
GET_PREF_FUNC_BOOL(	useNativeFileDialog,		Settings::USE_NATIVE_FILE_DIALOG					)
GET_PREF_FUNC_BOOL(	disableAnimations,			Settings::DISABLE_ANIMATIONS						)
GET_PREF_FUNC_BOOL(	generateMarkdown,			Settings::GENERATE_MARKDOWN_HELP					)
GET_PREF_FUNC_INT(	maxEngines,					Settings::MAX_ENGINE_COUNT							)
GET_PREF_FUNC_BOOL( windowsNoBomNative,			Settings::WINDOWS_NO_BOM_NATIVE						)

QString PreferencesModel::githubPatCustom() const
{
	return 	decrypt(Settings::value(Settings::GITHUB_PAT_CUSTOM).toString());
}


double PreferencesModel::uiScale()
{
	if (_uiScale < 0)
		_uiScale = Settings::value(Settings::UI_SCALE).toDouble();
	return _uiScale;
}

QStringList PreferencesModel::missingValues()		const
{
	QStringList items = Settings::value(Settings::MISSING_VALUES_LIST).toString().split("|");

	return items;
}

QStringList PreferencesModel::modulesRemembered()	const
{
	QStringList items = Settings::value(Settings::MODULES_REMEMBERED).toString().split("|");

	return items;
}

void PreferencesModel::moduleEnabledChanged(QString moduleName, bool enabled)
{
	QStringList list = modulesRemembered();

	if(list.contains(moduleName) != enabled)
	{
		if(enabled)	list.append(moduleName);
		else		list.removeAll(moduleName);
	}

	setModulesRemembered(list);
}

QString PreferencesModel::languageCode() const
{
	return LanguageModel::lang()->currentLanguageCode();
}

QString PreferencesModel::githubPatResolved() const
{
	if(githubPatUseDefault())
		return QProcessEnvironment::systemEnvironment().value("GITHUB_PAT", "@GITHUB_PAT_DEFINE@");
        // TODO: Make sure that I can pass the Github PAT

	return githubPatCustom();
}

QString PreferencesModel::fixedDecimalsForJS() const
{
	if(!fixedDecimals())
		return "\"\"";

	return QString::fromStdString(std::to_string(numDecimals()));
}

void PreferencesModel::setFixedDecimals(bool newFixedDecimals)
{
	if (fixedDecimals() == newFixedDecimals)
		return;

	Settings::setValue(Settings::FIXED_DECIMALS, newFixedDecimals);

	emit fixedDecimalsChanged(newFixedDecimals);
	emit fixedDecimalsChangedString(fixedDecimalsForJS());
}

void PreferencesModel::setNumDecimals(int newNumDecimals)
{
	if (numDecimals() == newNumDecimals)
		return;

	Settings::setValue(Settings::NUM_DECIMALS, newNumDecimals);

	emit numDecimalsChanged(newNumDecimals);

	if(fixedDecimals())
		emit fixedDecimalsChangedString(fixedDecimalsForJS());
}

void PreferencesModel::onUseDefaultPPIChanged(bool )
{
	if(customPPI() != defaultPPI())
		emit plotPPIChanged(plotPPI(), true);
}

void PreferencesModel::onCustomPPIChanged(int)
{
	if(!useDefaultPPI())
		emit plotPPIChanged(plotPPI(), true);
}

void PreferencesModel::onDefaultPPIChanged(int)
{

	if(useDefaultPPI())
		emit plotPPIChanged(plotPPI(), false);
}

#define SET_PREF_FUNCTION(TYPE, FUNC_NAME, GET_FUNC, NOTIFY, SETTING)	\
void PreferencesModel::FUNC_NAME(TYPE newVal)							\
{																		\
	if(GET_FUNC() == newVal) return;									\
	Settings::setValue(SETTING, newVal);								\
	emit NOTIFY(newVal);												\
}


SET_PREF_FUNCTION(bool,		setExactPValues,			exactPValues,				exactPValuesChanged,			Settings::EXACT_PVALUES								)
SET_PREF_FUNCTION(bool,		setNormalizedNotation,		normalizedNotation,			normalizedNotationChanged,		Settings::NORMALIZED_NOTATION						)
SET_PREF_FUNCTION(bool,		setDataAutoSynchronization, dataAutoSynchronization,	dataAutoSynchronizationChanged, Settings::DATA_AUTO_SYNCHRONIZATION					)
SET_PREF_FUNCTION(bool,		setUseDefaultEditor,		useDefaultEditor,			useDefaultEditorChanged,		Settings::USE_DEFAULT_SPREADSHEET_EDITOR			)
SET_PREF_FUNCTION(QString,	setCustomEditor,			customEditor,				customEditorChanged,			Settings::SPREADSHEET_EDITOR_NAME					)
SET_PREF_FUNCTION(bool,		setUseDefaultPPI,			useDefaultPPI,				useDefaultPPIChanged,			Settings::PPI_USE_DEFAULT							)
SET_PREF_FUNCTION(bool,		setDeveloperMode,			developerMode,				developerModeChanged,			Settings::DEVELOPER_MODE							)
SET_PREF_FUNCTION(QString,	setDeveloperFolder,			developerFolder,			developerFolderChanged,			Settings::DEVELOPER_FOLDER							)
SET_PREF_FUNCTION(int,		setCustomPPI,				customPPI,					customPPIChanged,				Settings::PPI_CUSTOM_VALUE							)
SET_PREF_FUNCTION(bool,		setLogToFile,				logToFile,					logToFileChanged,				Settings::LOG_TO_FILE								)
SET_PREF_FUNCTION(int,		setLogFilesMax,				logFilesMax,				logFilesMaxChanged,				Settings::LOG_FILES_MAX								)
SET_PREF_FUNCTION(int,		setMaxFlickVelocity,		maxFlickVelocity,			maxFlickVelocityChanged,		Settings::QML_MAX_FLICK_VELOCITY					)
SET_PREF_FUNCTION(bool,		setModulesRemember,			modulesRemember,			modulesRememberChanged,			Settings::MODULES_REMEMBER							)
SET_PREF_FUNCTION(QString,	setCranRepoURL,				cranRepoURL,				cranRepoURLChanged,				Settings::CRAN_REPO_URL								)
//SET_PREF_FUNCTION(QString,	setGithubPatCustom,			githubPatCustom,			githubPatCustomChanged,			Settings::GITHUB_PAT_CUSTOM							)
SET_PREF_FUNCTION(bool,		setGithubPatUseDefault,		githubPatUseDefault,		githubPatUseDefaultChanged,		Settings::GITHUB_PAT_USE_DEFAULT					)
SET_PREF_FUNCTION(QString,	setCurrentThemeName,		currentThemeName,			currentThemeNameChanged,		Settings::THEME_NAME								)
SET_PREF_FUNCTION(QString,	setPlotBackground,			plotBackground,				plotBackgroundChanged,			Settings::IMAGE_BACKGROUND							)
SET_PREF_FUNCTION(bool,		setUseNativeFileDialog,		useNativeFileDialog,		useNativeFileDialogChanged,		Settings::USE_NATIVE_FILE_DIALOG					)
SET_PREF_FUNCTION(bool,		setDisableAnimations,		disableAnimations,			disableAnimationsChanged,		Settings::DISABLE_ANIMATIONS						)
SET_PREF_FUNCTION(bool,		setGenerateMarkdown,		generateMarkdown,			generateMarkdownChanged,		Settings::GENERATE_MARKDOWN_HELP					)
SET_PREF_FUNCTION(QString,	setInterfaceFont,			interfaceFont,				interfaceFontChanged,			Settings::INTERFACE_FONT							)
SET_PREF_FUNCTION(QString,	setCodeFont,				codeFont,					codeFontChanged,				Settings::CODE_FONT									)
SET_PREF_FUNCTION(QString,	setResultFont,				resultFont,					resultFontChanged,				Settings::RESULT_FONT								)
SET_PREF_FUNCTION(int,		setMaxEngines,				maxEngines,					maxEnginesChanged,				Settings::MAX_ENGINE_COUNT							)
SET_PREF_FUNCTION(bool,		setWindowsNoBomNative,		windowsNoBomNative,			windowsNoBomNativeChanged,		Settings::WINDOWS_NO_BOM_NATIVE						)

void PreferencesModel::setGithubPatCustom(QString newPat)
{
	if (githubPatCustom() == newPat)
		return;

	Settings::setValue(Settings::GITHUB_PAT_CUSTOM, encrypt(newPat));

	emit githubPatCustomChanged();
}

void PreferencesModel::setWhiteBackground(bool newWhiteBackground)
{
	if (whiteBackground() == newWhiteBackground)
		return;

	setPlotBackground(newWhiteBackground ? "white" : "transparent");
}

void PreferencesModel::setDefaultPPI(int defaultPPI)
{
	if (_defaultPPI == defaultPPI)
		return;

	_defaultPPI = defaultPPI;
	emit defaultPPIChanged(_defaultPPI);
}

void PreferencesModel::setUiScale(double newUiScale)
{
	newUiScale = std::min(3.0, std::max(0.2, newUiScale));

	if (std::abs(uiScale() - newUiScale) < 0.001)
		return;

	Settings::setValue(Settings::UI_SCALE, newUiScale);
	_uiScale = newUiScale;

	emit uiScaleChanged(newUiScale);
}

void PreferencesModel::setModulesRemembered(QStringList newModulesRemembered)
{
	if (modulesRemembered() == newModulesRemembered)
		return;

	Settings::setValue(Settings::MODULES_REMEMBERED, newModulesRemembered.join('|'));
	emit modulesRememberedChanged();
}

void PreferencesModel::setSafeGraphics(bool newSafeGraphics)
{
	if (safeGraphics() == newSafeGraphics)
		return;

	Settings::setValue(Settings::SAFE_GRAPHICS_MODE, newSafeGraphics);
	emit modulesRememberChanged(newSafeGraphics);

	MessageForwarder::showWarning(tr("Safe Graphics mode changed"), tr("You've changed the Safe Graphics mode of JASP, for this option to take effect you need to restart JASP"));

	emit safeGraphicsChanged(newSafeGraphics);
}

void PreferencesModel::zoomIn()
{
	setUiScale(uiScale() + 0.1);
}

void PreferencesModel::zoomOut()
{
	if (uiScale() >= 0.2)
		setUiScale(uiScale() - 0.1);
}

void PreferencesModel::zoomReset()
{
	setUiScale(1.0);
}

void PreferencesModel::removeMissingValue(QString value)
{
	QStringList currentValues = missingValues();
	if(currentValues.contains(value))
	{
		currentValues.removeAll(value);
		Settings::setValue(Settings::MISSING_VALUES_LIST, currentValues.join("|"));
		emit missingValuesChanged();
	}
}

void PreferencesModel::addMissingValue(QString value)
{
	{
		QStringList currentValues = missingValues();
		if(!currentValues.contains(value))
		{
			currentValues.append(value);
			Settings::setValue(Settings::MISSING_VALUES_LIST, currentValues.join("|"));
			emit missingValuesChanged();
		}
	}
}

void PreferencesModel::resetMissingValues()
{
	QStringList currentValues = missingValues();
	Settings::setValue(Settings::MISSING_VALUES_LIST, Settings::defaultMissingValues);

	if(missingValues() != currentValues)
		emit missingValuesChanged();
}

void PreferencesModel::setCustomThresholdScale(bool newCustomThresholdScale)
{
	if (customThresholdScale() == newCustomThresholdScale)
		return;

	Settings::setValue(Settings::USE_CUSTOM_THRESHOLD_SCALE, newCustomThresholdScale);
	emit customThresholdScaleChanged (newCustomThresholdScale);
}

void PreferencesModel::setThresholdScale(int newThresholdScale)
{
	if (thresholdScale() == newThresholdScale)
		return;

	Settings::setValue(Settings::THRESHOLD_SCALE, newThresholdScale);
	emit thresholdScaleChanged(newThresholdScale);

}

void PreferencesModel::updateUtilsMissingValues()
{
	Utils::_currentEmptyValues = fq(missingValues());
	Utils::processEmptyValues();
}

void PreferencesModel::setCurrentThemeNameFromClass(JaspTheme * theme)
{
	if(theme)
		setCurrentThemeName(theme->themeName());
}

void PreferencesModel::onCurrentThemeNameChanged(QString newThemeName)
{
	JaspTheme::setCurrentThemeFromName(currentThemeName());
}


void PreferencesModel::_loadDatabaseFont()
{
	QFontDatabase fontDatabase;

	fontDatabase.addApplicationFont(":/fonts/FreeSans.ttf");
	fontDatabase.addApplicationFont(":/fonts/FiraCode-Retina.ttf");

	_allFonts = _allCodeFonts = _allResultFonts = _allInterfaceFonts = fontDatabase.families();
	_allCodeFonts.removeAll(defaultCodeFont());
	_allResultFonts.removeAll(defaultResultFont());
	_allInterfaceFonts.removeAll(defaultInterfaceFont());
}

QString PreferencesModel::_checkFontList(QString fonts) const
{
	if (fonts.contains(","))
		// If it is a list of fonts.
		// Select the first one which is available.
		for (QString font : fonts.split(","))
		{
			if (_allFonts.contains(font.remove('"')))
				return font;
		}

	return fonts;
}

QString PreferencesModel::defaultResultFont() const
{
	return _checkFontList(Settings::defaultValue(Settings::RESULT_FONT).toString());
}

QString PreferencesModel::resultFont(bool forWebEngine) const
{
	QString font		= Settings::value(Settings::RESULT_FONT).toString(),
			defaultFont = Settings::defaultValue(Settings::RESULT_FONT).toString();

	if (font.isEmpty()) font = defaultFont;

	if (forWebEngine)
	{
		// for WebEngine, if the font is the default one (that is a list of fonts), then use directly this list.
		// the css is then exacly the same as it used to be and we are sure that the user gets the same rendering.
		// If the font starts with a dot, then it needs extra quote for the WebEngine.
		if (font.startsWith("."))
			font = '"' + font + '"';
	}
	else
		font = _checkFontList(font);

	return font;
}

QString PreferencesModel::interfaceFont() const
{
	QString font = Settings::value(Settings::INTERFACE_FONT).toString();

	if (font.isEmpty()) font = defaultInterfaceFont();

	return font;
}

QString PreferencesModel::codeFont() const
{
	QString font = Settings::value(Settings::CODE_FONT).toString();

	if (font.isEmpty()) font = defaultCodeFont();

	return font;
}

QString PreferencesModel::defaultInterfaceFont() const
{
	return Settings::defaultValue(Settings::INTERFACE_FONT).toString();
}

QString PreferencesModel::defaultCodeFont() const
{
	return Settings::defaultValue(Settings::CODE_FONT).toString();
}

void PreferencesModel::resetRememberedModules(bool setToRemember) 
{
	setModulesRemembered(!setToRemember ? QStringList({}) : RibbonModel::singleton()->getModulesEnabled());
}

void PreferencesModel::setLcCtypeWin(int lcCtypeWin)
{
	winLcCtypeSetting	current = Settings::getWinLcCtypeSetting(),
						newSet	= winLcCtypeSetting(lcCtypeWin);
	
	if(current != newSet)
	{
		Settings::setValue(Settings::LC_CTYPE_C_WIN, winLcCtypeSettingToQString(newSet));
		
		emit lcCtypeChanged();
		emit restartAllEngines();
	}
}

int PreferencesModel::lcCtypeWin() const
{
	winLcCtypeSetting	current = Settings::getWinLcCtypeSetting();
	
	return static_cast<int>(current);
}

bool PreferencesModel::setLC_CTYPE_C() const
{
#ifndef _WIN32
	//throw std::runtime_error("PreferencesModel::setLC_CTYPE_C() should only be used on Windows.");
	return false; //The result is interpreted as "force LC_CTYPE to C". I guess the "neverC" enum is also a bit misleading then... But all of this can be thrown away once utf-8 is supported on win.
#endif
	
	switch(Settings::getWinLcCtypeSetting())
	{
	case winLcCtypeSetting::check:		return pathIsSafeForR(AppDirs::rHome());
	case winLcCtypeSetting::neverC:		return false;
	case winLcCtypeSetting::alwaysC:	return true;
	}
}
