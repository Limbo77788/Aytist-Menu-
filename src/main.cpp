#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <imgui-cocos.hpp>

using namespace geode::prelude;

// Глобальные переменные чита
bool g_noclip = false;
bool g_showHitboxes = false;
float g_speedhack = 1.0f;

// Хук на смерть игрока
class $modify(PlayerObject) {
    void playDeathEffect() {
        if (!g_noclip) {
            PlayerObject::playDeathEffect();
        }
    }
};

// Хук на игровой слой
class $modify(PlayLayer) {
    void update(float dt) {
        dt *= g_speedhack;
        PlayLayer::update(dt);
    }

    void draw() {
        PlayLayer::draw();
        if (g_showHitboxes && m_player1) {
            // Отрисовка хитбокса игрока
            auto pos = m_player1->getPosition();
            CCDrawNode::create()->drawCircle(pos, 15, 0, 360, false, ccc4f(1, 0, 0, 1));
            
            // Хитбоксы препятствий
            for (auto obj : *m_objects) {
                if (obj->m_isSolid) {
                    auto rect = obj->getBoundingBox();
                    CCDrawNode::create()->drawRect(rect.origin, rect.origin + rect.size, ccc4f(0, 1, 0, 0.5f));
                }
            }
        }
    }
};

// Редактор статистики (реальный, с сохранением)
class StatEditor : public Popup<> {
protected:
    TextInput *m_demonInput, *m_starInput, *m_moonInput, *m_coinInput, *m_orbInput;

    bool setup() override {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto title = CCLabelBMFont::create("AYTIST STAT EDITOR", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height - 30);
        m_mainLayer->addChild(title);

        // Демоны
        m_demonInput = TextInput::create(200, "Demons");
        m_demonInput->setPosition(winSize.width / 2, winSize.height - 80);
        m_demonInput->setString(std::to_string(GameStatsManager::sharedState()->getStat("5")));
        m_mainLayer->addChild(m_demonInput);

        // Звёзды
        m_starInput = TextInput::create(200, "Stars");
        m_starInput->setPosition(winSize.width / 2, winSize.height - 120);
        m_starInput->setString(std::to_string(GameManager::sharedState()->getPlayerStars()));
        m_mainLayer->addChild(m_starInput);

        // Луны
        m_moonInput = TextInput::create(200, "Moons");
        m_moonInput->setPosition(winSize.width / 2, winSize.height - 160);
        m_moonInput->setString(std::to_string(GameStatsManager::sharedState()->getStat("28")));
        m_mainLayer->addChild(m_moonInput);

        // Секретные монеты
        m_coinInput = TextInput::create(200, "Secret Coins");
        m_coinInput->setPosition(winSize.width / 2, winSize.height - 200);
        m_coinInput->setString(std::to_string(GameManager::sharedState()->getPlayerSecretCoins()));
        m_mainLayer->addChild(m_coinInput);

        // Орбы
        m_orbInput = TextInput::create(200, "Orbs");
        m_orbInput->setPosition(winSize.width / 2, winSize.height - 240);
        m_orbInput->setString(std::to_string(GameManager::sharedState()->getPlayerOrbs()));
        m_mainLayer->addChild(m_orbInput);

        // Кнопка Apply
        auto applyBtn = ButtonSprite::create("Apply");
        auto applyButton = CCMenuItemSpriteExtra::create(
            applyBtn, this, menu_selector(StatEditor::onApply)
        );
        applyButton->setPosition(winSize.width / 2, 40);

        auto menu = CCMenu::create(applyButton, nullptr);
        menu->setPosition(0, 0);
        m_mainLayer->addChild(menu);

        return true;
    }

    void onApply(CCObject*) {
        // Применяем изменения
        GameStatsManager::sharedState()->setStat("5", std::stoi(m_demonInput->getString()));
        GameManager::sharedState()->setPlayerStars(std::stoi(m_starInput->getString()));
        GameStatsManager::sharedState()->setStat("28", std::stoi(m_moonInput->getString()));
        GameManager::sharedState()->setPlayerSecretCoins(std::stoi(m_coinInput->getString()));
        GameManager::sharedState()->setPlayerOrbs(std::stoi(m_orbInput->getString()));

        // Сохраняем в файл
        GameManager::sharedState()->save();
        GameStatsManager::sharedState()->saveData();

        FLAlertLayer::create("Aytist", "Stats Updated! Restart game to see changes.", "OK")->show();
    }

public:
    static StatEditor* create() {
        auto ret = new StatEditor();
        if (ret && ret->init(300, 350)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// Добавляем кнопку в главное меню и меню паузы
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Кнопка редактора статы
        auto statBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png"),
            this,
            menu_selector(MenuLayer::onStatEditor)
        );
        statBtn->setPosition(100, 100);
        m_buttonMenu->addChild(statBtn);

        // ImGui меню
        ImGuiCocos::get().setup([]() {
            ImGui::Begin("AYTIST MENU");
            ImGui::Checkbox("Noclip", &g_noclip);
            ImGui::Checkbox("Show Hitboxes", &g_showHitboxes);
            ImGui::SliderFloat("Speedhack", &g_speedhack, 0.1f, 5.0f);
            ImGui::End();
        });

        return true;
    }

    void onStatEditor(CCObject*) {
        StatEditor::create()->show();
    }
};

// Добавляем кнопку редактора в паузу
class $modify(PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png"),
            this,
            menu_selector(PauseLayer::onStatEditor)
        );
        btn->setPosition(-200, 0);
        m_buttonMenu->addChild(btn);
    }

    void onStatEditor(CCObject*) {
        StatEditor::create()->show();
    }
};
