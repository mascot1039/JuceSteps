#include "Multi10KeyButton.h"

Multi10KeyButton::Multi10KeyButton (int number)
{
    // LookAndFeel を安全に生成して保持
    buttonLookAndFeel = std::make_unique<Multi10KeyButtonLookAndFeel> (number);

    // 自身の LookAndFeel として適用
    setLookAndFeel (buttonLookAndFeel.get());

    // 境界線の接続設定をリセット
    setConnectedEdges (0);
}

Multi10KeyButton::~Multi10KeyButton()
{
    // 重要: 自身が破棄される前に、LookAndFeel の参照を nullptr にして安全に解除します
    setLookAndFeel (nullptr);
}

void Multi10KeyButton::setDrawMode(Multi10KeyButtonLookAndFeel::Multi10KeyDrawMode newMode)
{
    if (buttonLookAndFeel != nullptr)
    {
        buttonLookAndFeel->setDrawMode (newMode);
        repaint(); // 💡 モードが変わったら再描画を要求する
    }
}
