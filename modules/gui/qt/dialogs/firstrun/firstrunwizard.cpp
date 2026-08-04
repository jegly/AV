/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "firstrunwizard.hpp"
#include "util/color_scheme_model.hpp"
#include "style/medeapalettes.hpp"
#include "maininterface/mainctx.hpp"
#include "dialogs/toolbar/controlbar_profile_model.hpp"
#include "medialibrary/medialib.hpp"

#include <QPushButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QComboBox>
#include <QLabel>

#include <vlc_common.h>
#include <vlc_configuration.h>
#include <vlc_url.h>
#include <vlc_cxx_helpers.hpp>

FirstRunWizard::FirstRunWizard( qt_intf_t *_p_intf, QWidget *parent)
               : QWizard( parent )
{
    p_intf = _p_intf;

    /* Set windown properties */
    setWindowTitle( qtr( "Welcome" ) );
    setWindowModality( Qt::WindowModal );

    // NOTE: Aero style seems to be incompatible with
    //       Qt Modern Windows Style dark mode
    setWizardStyle(QWizard::ModernStyle);

    /* Build the Ui */
    ui.setupUi( this );

    /* Set the privacy and network policy */
    ui.policy->setHtml( qtr( "<p><i>AV</i> does <b>not</b> collect personal "
        "data, and does not transmit anything to anyone."
        "</p>\n"
        "<p>It also cannot. Every module capable of opening a network socket "
        "is removed from the build, and the package does not request network "
        "access, so it is denied by the system regardless of what the "
        "application does."
        "</p>\n"
        "<p>This means cover art, track names and other metadata are never "
        "fetched from Internet services. Only what is already in your files "
        "is shown."
        "</p>\n" ) );
    ui.policy->setReadOnly( true );

    /* Set up the button group for colour schemes. */
    /* Creating in Qt Designer has unstable ordering when calling buttons() */
    /* Medea has no System/Day/Night tri-state - it has a palette list. The
     * wizard offers three starting points from that list; the rest are in
     * preferences. IDs are resolved by key rather than hardcoded, so adding or
     * reordering palettes cannot silently repoint these buttons. */
    colorSchemeGroup = new QButtonGroup( this );
    colorSchemeGroup->addButton( ui.systemButton, medea_palette_index( "ptyxis_nord" ) );
    colorSchemeGroup->addButton( ui.lightButton, medea_palette_index( "ptyxis_kanagawa" ) );
    colorSchemeGroup->addButton( ui.darkButton, medea_palette_index( "ptyxis_synthwave" ) );
    colorSchemeGroup->setExclusive( true );

    /* The three radio buttons are only shortcuts. Medea ships 49 palettes and
     * the Ptyxis set is the point of it, so the full list is offered here
     * rather than being reachable only after first launch. */
    paletteCombo = new QComboBox( this );
    for( int i = 0; i < medea_palettes_count; i++ )
        paletteCombo->addItem( QString::fromUtf8( medea_palettes[i].displayName ), i );

    {
        const int defaultIdx = medea_palette_index( MEDEA_DEFAULT );
        if( defaultIdx >= 0 )
            paletteCombo->setCurrentIndex( defaultIdx );
    }

    connect( paletteCombo, &QComboBox::currentIndexChanged, this, [this]( int row ) {
        if( row < 0 || row >= medea_palettes_count )
            return;
        ui.explainerLabel->setText( qtr( "<i>AV will use the %1 theme.</i>" )
                                    .arg( QString::fromUtf8( medea_palettes[row].displayName ) ) );
    } );

    ui.gridLayout_3->addWidget( new QLabel( qtr( "All themes:" ), this ), 4, 0 );
    ui.gridLayout_3->addWidget( paletteCombo, 4, 1, 1, 2 );

    colorSchemeImages = new QButtonGroup( this );
    colorSchemeImages->addButton( ui.daynightImage );
    colorSchemeImages->addButton( ui.lightImage );
    colorSchemeImages->addButton( ui.darkImage );

    /* Setup the layout page */
    ui.layoutGroup->setId( ui.modernButton, 0 );
    ui.layoutGroup->setId( ui.classicButton, 1 );

    layoutImages = new QButtonGroup( this );
    layoutImages->addButton( ui.modernImage );
    layoutImages->addButton( ui.classicImage );
    layoutImages->setId( ui.modernImage, MODERN );
    layoutImages->setId( ui.classicImage, CLASSIC );

    /* Set the tooltips for each of the layout choices */
    ui.classicButton->setToolTip( qtr("<ul>"
                                      "<li>No client-side decoration</li>"
                                      "<li>Toolbar menu</li>"
                                      "<li>Pinned video controls</li>"
                                      "</ul>") );

    ui.modernButton->setToolTip( qtr("<ul>"
                                     "<li>Client-side decoration</li>"
                                     "<li>No toolbar menu</li>"
                                     "<li>Video controls unpinned</li>"
                                     "</ul>") );

    /* Remove the cancel button */
    setOption( QWizard::NoCancelButton );

    /* Create new instance of MLFoldersModel */
    if ( vlc_ml_instance_get( p_intf ) )
    {
        const auto foldersModel = new MLFoldersModel( this );
        foldersModel->setCtx( p_intf->p_mi );
        ui.entryPoints->setMLFoldersModel( foldersModel );
        mlFoldersEditor = ui.entryPoints;
        mlFoldersModel = foldersModel;
    }
    else
    {
        ui.enableMl->setChecked( false );
    }

    /* Disable the ML button as toggling doesn't actually do anything */
    ui.enableMl->setEnabled(false);

    /* Slots and Signals */
    connect( ui.addButton, &QPushButton::clicked, this, &FirstRunWizard::MLaddNewFolder );
    connect( colorSchemeGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::updateColorLabel );
    connect( colorSchemeImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::imageColorSchemeClick );
    connect( ui.layoutGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::updateLayoutLabel );
    connect( layoutImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::imageLayoutClick );
    connect( this->button(QWizard::FinishButton), &QPushButton::clicked, this, &FirstRunWizard::finish );
}

/**
 * Function called when the finish button is pressed.
 * Processes all inputted settings according to chosen options
 */
void FirstRunWizard::finish()
{
    /* Welcome Page settings  */
    config_PutInt( "metadata-network-access", ui.privacyCheckbox->isChecked() );
    config_PutInt( "qt-privacy-ask", 0 );

    /* Colour Page settings */
    p_intf->p_mi->getColorScheme()->setCurrentIndex(
        paletteCombo ? paletteCombo->currentIndex() : 0 );

    /* Layout Page settings */
    config_PutInt( "qt-menubar", ui.layoutGroup->checkedId() );
    /* Medea always draws its own title bar so the window chrome follows the
     * palette; the layout choice must not switch it back to the system one. */
    config_PutInt( "qt-titlebar", 0 );

    config_PutInt( "qt-pin-controls", ui.layoutGroup->checkedId() );

    {
        ControlbarProfileModel* controlbarModel = p_intf->p_mi->controlbarProfileModel();
        assert(controlbarModel);

        ControlbarProfileModel::Style style;
        if( ui.layoutGroup->checkedId() )
            style = ControlbarProfileModel::Style::CLASSIC_STYLE;
        else
            style = ControlbarProfileModel::Style::DEFAULT_STYLE;

        std::unique_ptr<ControlbarProfile> profile( controlbarModel->generateProfileFromStyle( style ) );
        assert( profile );
        const std::optional<int> index = controlbarModel->findModel( profile.get() );
        if( index )
            controlbarModel->setSelectedProfile( *index );
        else
            controlbarModel->insertProfile( std::move( profile ), true );
    }

    /* Commit changes to the scanned folders for the Media Library */
    if( vlc_ml_instance_get( p_intf ) && mlFoldersEditor )
        mlFoldersEditor->commit();

    /* Reload the indexing service for the media library */
    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();
    config_SaveConfigFile( p_intf );
}

/**
 * Opens a QFileDialog for the user to select a folder to add to the Media Library
 * If a folder is selected, add it to the list of folders under MLFoldersEditor
 */
void FirstRunWizard::MLaddNewFolder()
{
    QUrl newEntryPoint = QFileDialog::getExistingDirectoryUrl( this, qtr("Choose a folder to add to the Media Library"),
                                                               QUrl( QDir::homePath() ));

    if( !newEntryPoint.isEmpty() && !mlFoldersEditor->contains( newEntryPoint ) )
        mlFoldersEditor->add( newEntryPoint );
}

/**
 * Automatically updates the label on the color scheme page depending
 * on the currently selected radio button choice
 * @param id The id of the button we are updating the label of
 */
void FirstRunWizard::updateColorLabel( QAbstractButton* btn )
{
    const int idx = colorSchemeGroup->id( btn );
    if ( idx < 0 || idx >= medea_palettes_count )
        return;

    if ( paletteCombo )
        paletteCombo->setCurrentIndex( idx );

    ui.explainerLabel->setText( qtr( "<i>AV will use the %1 theme. "
                                     "More are available in Preferences.</i>" )
                                .arg( QString::fromUtf8( medea_palettes[idx].displayName ) ) );
}

/**
 * Automatically updates the label depending on the currently selected layout
 * @param id The id of the button we are updating the label of
 */
void FirstRunWizard::updateLayoutLabel( QAbstractButton* btn )
{
    switch ( ui.layoutGroup->id( btn ) )
    {
        case MODERN:
            ui.layoutExplainer->setText( qtr( "<i>AV will use a modern layout with no menubar or pinned controls but with client-side decoration</i>" ) );
            break;
        case CLASSIC:
            ui.layoutExplainer->setText( qtr( "<i>AV will use a classic layout with a menubar and pinned controls but with no client-side decoration</i>" ) );
            break;
    }
}

/**
 * Checks the correct button when the corresponding image is clicked
 * for the color scheme page.
 * @param id The id of the image that was clicked
 */
void FirstRunWizard::imageColorSchemeClick( QAbstractButton* btn )
{
    QAbstractButton* groupBtn = colorSchemeGroup->buttons().at( colorSchemeImages->id( btn ) );
    assert( groupBtn );
    groupBtn->setChecked( true );
    updateColorLabel( groupBtn );
}

/**
 * Checks the correct button when the corresponding image is clicked
 * for the layouts page.
 * @param id The id of the image that was clicked
 */
void FirstRunWizard::imageLayoutClick( QAbstractButton* btn )
{
    QAbstractButton* layoutBtn = ui.layoutGroup->buttons().at( layoutImages->id( btn ) );
    assert( layoutBtn );
    layoutBtn->setChecked( true );
    updateLayoutLabel( layoutBtn );
}

/**
 * Defines the navigation of pages for the wizard
 * @return int - the page id to go to or -1 if we are done
 */
int FirstRunWizard::nextId() const
{
    switch ( currentId() )
    {
        case WELCOME_PAGE:
            if( ui.enableMl->isChecked() )
                return FOLDER_PAGE;
            else
                return COLOR_SCHEME_PAGE;
        case FOLDER_PAGE:
            return COLOR_SCHEME_PAGE;
        case COLOR_SCHEME_PAGE:
            return LAYOUT_PAGE;
        default:
            return -1;
    }
}

/**
 * Sets up the buttons and options for the color scheme page
 * Needs to be set up later or else the main interface hasn't been created yet
 * Color schemes are always in the order system/auto, day then night
 */
void FirstRunWizard::initializePage( int id )
{
    if(id == COLOR_SCHEME_PAGE)
    {
        /* Medea exposes 49 palettes but the wizard only offers three starting
         * points, so buttons are deliberately NOT 1:1 with the scheme list -
         * the upstream assert on equal sizes cannot hold here. The IDs were
         * already assigned by palette key in the constructor; this only puts
         * the palette's display name on each button. */
        const auto schemeButtons = colorSchemeGroup->buttons();
        const auto schemeImages = colorSchemeImages->buttons();

        for( QAbstractButton *btn : schemeButtons )
        {
            const int idx = colorSchemeGroup->id( btn );
            if( idx >= 0 && idx < medea_palettes_count )
                btn->setText( QString::fromUtf8( medea_palettes[idx].displayName ) );
        }

        /* Keep the decorative image buttons pointing at the same palettes. */
        for( int i = 0; i < schemeImages.size() && i < schemeButtons.size(); i++ )
            colorSchemeImages->setId( schemeImages.at(i),
                                      colorSchemeGroup->id( schemeButtons.at(i) ) );

        if( !schemeButtons.isEmpty() )
            updateColorLabel( schemeButtons.first() );
    }
    else if ( id == FOLDER_PAGE )
        addDefaults();
}

/**
 * Processes the default options on rejection of the FirstRun Wizard.
 * The default options are:
 * - Yes to metadata
 * - System/Auto colour scheme
 * - Modern VLC layout
 */
void FirstRunWizard::reject()
{
    assert(p_intf->p_mi);
    /* Welcome Page settings  */
    config_PutInt( "metadata-network-access", 1 );
    config_PutInt( "qt-privacy-ask", 0 );

    /* Colour Page settings */
    p_intf->p_mi->getColorScheme()->setCurrentIndex( medea_palette_index( MEDEA_DEFAULT ) );

    /* Layout Page settings */
    config_PutInt( "qt-menubar", 0 );
    config_PutInt( "qt-titlebar", 0 );
    p_intf->p_mi->setPinVideoControls( 0 );

    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    config_SaveConfigFile( p_intf );
    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();

    done( QDialog::Rejected );
}

/**
 * Adds the default folders to the media library
 */
void FirstRunWizard::addDefaults()
{
    // Return if we already set the defaults or something is null
    if( mlDefaults || !mlFoldersEditor || mlFoldersEditor->rowCount() )
        return;

    for( auto&& target : { VLC_VIDEOS_DIR, VLC_MUSIC_DIR } )
    {
        auto folder = vlc::wrap_cptr( config_GetUserDir( target ) );
        if( folder == nullptr )
            continue;
        auto folderMrl = vlc::wrap_cptr( vlc_path2uri( folder.get(), nullptr ) );
        if ( !mlFoldersEditor->contains( QUrl( folderMrl.get() ) ) )
            mlFoldersEditor->add( QUrl( folderMrl.get() ) );
    }

    mlDefaults = true;
}
