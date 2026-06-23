#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
// 1. Custom Button Class for the Window Controls
//==============================================================================
class MacTrafficLightButton : public juce::Button
{
public:
    MacTrafficLightButton(juce::Colour c) : juce::Button("CustomBtn"), colour(c)
    {
        // Explicitly demand a redraw on mouse activity, bypassing Release optimizations
        setRepaintsOnMouseActivity(true);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Colour drawColour = colour;

        // Add hard fallback checks (isMouseButtonDown / isMouseOver)
        // to guarantee the state updates even if the standard flags are throttled.
        if (shouldDrawButtonAsDown || isMouseButtonDown())
            drawColour = drawColour.darker(0.2f);
        else if (shouldDrawButtonAsHighlighted || isMouseOver())
            drawColour = drawColour.brighter(0.4f);

        auto bounds = getLocalBounds().toFloat().reduced(4.0f);
        float diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
        juce::Rectangle<float> circle(bounds.getCentreX() - diameter / 2.0f,
                                      bounds.getCentreY() - diameter / 2.0f,
                                      diameter, diameter);

        g.setColour(drawColour);
        g.fillEllipse(circle);
    }

    // Force an absolute UI refresh the exact moment the cursor crosses the boundary
    void mouseEnter(const juce::MouseEvent& e) override { juce::Button::mouseEnter(e); repaint(); }
    void mouseExit(const juce::MouseEvent& e)  override { juce::Button::mouseExit(e);  repaint(); }

private:
    juce::Colour colour;
};

//==============================================================================
// 2. Custom LookAndFeel to inject the buttons & blend the title bar
//==============================================================================
class MacLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MacLookAndFeel() {}

    // Intercept window button creation (Now using monochrome grey)
    juce::Button* createDocumentWindowButton(int buttonType) override
    {
        // 0xff3e3e3e is a shade lighter than the 0xff1e1e1e background
        juce::Colour subtleGrey(0xff3e3e3e);

        if (buttonType == juce::DocumentWindow::closeButton)
            return new MacTrafficLightButton(subtleGrey);

        if (buttonType == juce::DocumentWindow::minimiseButton)
            return new MacTrafficLightButton(subtleGrey);

        if (buttonType == juce::DocumentWindow::maximiseButton)
            return new MacTrafficLightButton(subtleGrey);

        return juce::LookAndFeel_V4::createDocumentWindowButton(buttonType);
    }

    // Intercept title bar drawing to make it blend perfectly
    void drawDocumentWindowTitleBar (juce::DocumentWindow& window, juce::Graphics& g,
                                     int w, int h, int titleSpaceX, int titleSpaceW,
                                     const juce::Image* icon, bool drawTitleTextOnLeft) override
    {
        // 1. Fill completely flat with the app's background color
        g.fillAll(juce::Colour(0xff1e1e1e));

        // 2. Draw the window title subtly so you still know what app you are in
        g.setColour(juce::Colour(0xff555555)); // Muted dark grey
        g.setFont(14.0f);
        g.drawText(window.getName(), 0, 0, w, h, juce::Justification::centred);
    }
};

//==============================================================================
// 3. Main Application Wrapper
//==============================================================================
class SetlistMetronomeApplication : public juce::JUCEApplication
{
public:
    SetlistMetronomeApplication() {}

    const juce::String getApplicationName() override       { return "Setlist Metronome"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name, juce::Colour(0xff1e1e1e), DocumentWindow::allButtons)
        {
            // 1. Disable the ugly Windows native title bar
            setUsingNativeTitleBar(false);

            // 2. Force buttons to the RIGHT side (set to false)
            setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);

            // 3. Apply the custom styling
            setLookAndFeel(&macLookAndFeel);

            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        ~MainWindow() override
        {
            // Always clear the custom LookAndFeel before destroying the window
            setLookAndFeel(nullptr);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        MacLookAndFeel macLookAndFeel;
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(SetlistMetronomeApplication)