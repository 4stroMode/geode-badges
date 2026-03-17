#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <string>
#include <map>
#include <regex>
#include <sstream>
#include <thread>

using namespace geode::prelude;

// mapping for usernames -> badge ids
std::map<std::string, int> g_playerBadges;
bool g_badgesLoaded = false;

// returns the texture name based on badge id
std::string getBadgeTextureName(int badgeId) {
    switch (badgeId) {
        case 1:  return "ownerbadge";
        case 2:  return "michigunbadge";
        case 3:  return "ytbadge1";
        case 4:  return "ytbadge2";
        case 5:  return "ytbadge3";
        case 6:  return "tbadge1";
        case 7:  return "tbadge2";
        case 8:  return "tbadge3";
        case 9:  return "xbadge1";
        case 10: return "xbadge2";
        case 11: return "xbadge3";
        case 12: return "creatorbadge";
        case 13: return "playerbadge";
        default: return "";
    }
}

void fetchBadge(int badgeId, const std::string& filename) {
    std::string url = "https://astrostudios.neocities.org/badges/" + filename;
    std::thread([badgeId, url]() {
        auto res = geode::utils::web::WebRequest().getSync(url);
        if (res.ok()) {
            auto result = res.string();
            if (result.isOk()) {
                std::string data = result.unwrap();
                std::istringstream stream(data);
                std::string line;
                while (std::getline(stream, line)) {
                    line.erase(line.find_last_not_of(" \n\r\t") + 1);
                    line.erase(0, line.find_first_not_of(" \n\r\t"));
                    if (!line.empty()) {
                        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                        geode::queueInMainThread([line, badgeId]() {
                            g_playerBadges[line] = badgeId;
                        });
                    }
                }
            }
        } else {
            log::warn("Failed to fetch badges from {}: error code {}", url, res.code());
        }
    }).detach();
}

$on_mod(Loaded) {
    g_playerBadges.clear();
    g_badgesLoaded = true;
    fetchBadge(1, "owner.txt");
    fetchBadge(2, "michigun.txt");
    fetchBadge(3, "youtube1.txt");
    fetchBadge(4, "youtube2.txt");
    fetchBadge(5, "youtube3.txt");
    fetchBadge(6, "twitch1.txt");
    fetchBadge(7, "twitch2.txt");
    fetchBadge(8, "twitch3.txt");
    fetchBadge(9, "x1.txt");
    fetchBadge(10, "x2.txt");
    fetchBadge(11, "x3.txt");
    fetchBadge(12, "creator.txt");
    fetchBadge(13, "player.txt");
    log::info("Started fetching badges from neocities.");
}

void showBadgeInfo(int badgeId) {
    std::string title = "Player Badge";
    std::string desc = "This user holds a special community badge!";
    
    switch (badgeId) {
        case 1:  title = "Owner"; desc = "Owner of Badges for Players"; break;
        case 2:  title = "Legend"; desc = "/\\/\\/\\"; break;
        case 3:  title = "YouTube Level 1"; desc = "This user is a recognized content creator on YouTube."; break;
        case 4:  title = "YouTube Level 2"; desc = "This user is an outstanding content creator on YouTube."; break;
        case 5:  title = "YouTube Level 3"; desc = "This user is a legendary content creator on YouTube."; break;
        case 6:  title = "Twitch Level 1"; desc = "This user is a recognized streamer on Twitch."; break;
        case 7:  title = "Twitch Level 2"; desc = "This user is an outstanding streamer on Twitch."; break;
        case 8:  title = "Twitch Level 3"; desc = "This user is a legendary streamer on Twitch."; break;
        case 9:  title = "X Level 1"; desc = "This user is active in the community on X/Twitter."; break;
        case 10: title = "X Level 2"; desc = "This user provides great community updates on X/Twitter."; break;
        case 11: title = "X Level 3"; desc = "This user is a highly influential community figure on X/Twitter."; break;
        case 12: title = "Creator"; desc = "This user is a recognized Geometry Dash level creator."; break;
        case 13: title = "Player"; desc = "This user is a recognized top Geometry Dash player."; break;
    }
    
    FLAlertLayer::create(title.c_str(), desc.c_str(), "OK")->show();
}

class $modify(MyProfilePage, ProfilePage) {
    void onCustomBadge(CCObject* sender) {
        if (auto node = typeinfo_cast<CCNode*>(sender)) {
            showBadgeInfo(node->getTag());
        }
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        
        if (!g_badgesLoaded || !score) return;

        std::string username = score->m_userName;
        std::transform(username.begin(), username.end(), username.begin(), ::tolower);

        if (g_playerBadges.contains(username)) {
            int badgeId = g_playerBadges[username];
            std::string textureNameBase = getBadgeTextureName(badgeId);
            
            if (textureNameBase.empty()) return;

            auto layer = m_mainLayer;
            if (auto username_menu = static_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"))) {
                // duplicate prevention
                if (auto existingBadge = username_menu->getChildByID("custom-player-badge"_spr)) {
                    existingBadge->removeFromParent();
                }

                std::string textureName;
                float factor = CCDirector::sharedDirector()->getContentScaleFactor();
                float targetWidth = 25.0f;
                float finalScale = 1.0f;
                
                // load textures based on quality factor
                if (factor >= 4.0f) {
                    textureName = textureNameBase + "-uhd.png";
                } else if (factor >= 2.0f) {
                    textureName = textureNameBase + "-hd.png";
                } else {
                    textureName = textureNameBase + ".png";
                }

                std::string exactPath = fmt::format("{}/{}", Mod::get()->getID(), textureName);
                if (auto badgeSprite = CCSprite::create(exactPath.c_str())) {
                    // Calculamos la escala idónea dependiendo del tamaño real de esta imagen específica
                    finalScale = targetWidth / badgeSprite->getContentSize().width;
                    badgeSprite->setScale(finalScale);

                    auto badgeBtn = CCMenuItemSpriteExtra::create(
                        badgeSprite,
                        this,
                        menu_selector(MyProfilePage::onCustomBadge)
                    );
                    
                    badgeBtn->setTag(badgeId);
                    badgeBtn->setID("custom-player-badge"_spr);
                    username_menu->addChild(badgeBtn);
                    username_menu->updateLayout();
                }
            }
        }
    }
};

class $modify(MyCommentCell, CommentCell) {
    void onCustomBadge(CCObject* sender) {
        if (auto node = typeinfo_cast<CCNode*>(sender)) {
            showBadgeInfo(node->getTag());
        }
    }

    void loadFromComment(GJComment* comment) {
        CommentCell::loadFromComment(comment);
        
        if (!g_badgesLoaded || !comment) return;

        std::string username = comment->m_userName;
        std::transform(username.begin(), username.end(), username.begin(), ::tolower);

        if (g_playerBadges.contains(username)) {
            int badgeId = g_playerBadges[username];
            std::string textureNameBase = getBadgeTextureName(badgeId);
            
            if (textureNameBase.empty()) return;

            auto layer = m_mainLayer;
            if (auto username_menu = static_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"))) {
                // duplicate prevention
                if (auto existingBadge = username_menu->getChildByID("custom-comment-badge"_spr)) {
                    existingBadge->removeFromParent();
                }

                std::string textureName;
                float factor = CCDirector::sharedDirector()->getContentScaleFactor();
                float targetWidth = 15.0f;
                float finalScale = 1.0f;
                
                // load textures based on quality factor
                if (factor >= 4.0f) {
                    textureName = textureNameBase + "-uhd.png";
                } else if (factor >= 2.0f) {
                    textureName = textureNameBase + "-hd.png";
                } else {
                    textureName = textureNameBase + ".png";
                }

                std::string exactPath = fmt::format("{}/{}", Mod::get()->getID(), textureName);
                if (auto badgeSprite = CCSprite::create(exactPath.c_str())) {
                    finalScale = targetWidth / badgeSprite->getContentSize().width;
                    badgeSprite->setScale(finalScale);
                    
                    auto badgeBtn = CCMenuItemSpriteExtra::create(
                        badgeSprite,
                        this,
                        menu_selector(MyCommentCell::onCustomBadge)
                    );
                    badgeBtn->setTag(badgeId);
                    badgeBtn->setID("custom-comment-badge"_spr);
                    username_menu->addChild(badgeBtn);
                    
                }
            }

            // apply custom comment colors if applicable
            ccColor3B commentColor = {255, 255, 255};
            bool shouldColor = false;

            if (badgeId == 1) { // Owner
                commentColor = {3,252,252}; // Azul claro
                shouldColor = true;
            } else if (badgeId == 5) { // YT Lvl 3
                commentColor = {255, 50, 50}; // Rojo
                shouldColor = true;
            } else if (badgeId == 8) { // Twitch Lvl 3
                commentColor = {232, 163, 255}; // Morado
                shouldColor = true;
            } else if (badgeId == 11) { // X Lvl 3
                commentColor = {50, 50, 50}; // Negro / Gris Oscuro
                shouldColor = true;
            } else if (badgeId == 12 || badgeId == 13) { // Creator o Player
                commentColor = {255, 255, 50}; // Amarillo
                shouldColor = true;
            }

            if (shouldColor) {
                if (auto textLabel = typeinfo_cast<SimpleTextArea*>(layer->getChildByIDRecursive("comment-text-label"))) {
                    textLabel->setColor({commentColor.r, commentColor.g, commentColor.b, 255});
                } else if (auto bmLabel = typeinfo_cast<CCLabelBMFont*>(layer->getChildByIDRecursive("comment-text-label"))) {
                    bmLabel->setColor(commentColor);
                }
            }
        }
    }
};
