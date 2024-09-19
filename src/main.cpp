
#include <Geode/modify/PlayLayer.hpp>
#include <OpenRGB/DeviceInfo.hpp>
#include <cstdint>


#include "Geode/binding/GameManager.hpp"
#include "Geode/binding/PlayLayer.hpp"
#include "Geode/loader/Mod.hpp"
#include "OpenRGB/Client.hpp"
#include "ccTypes.h"


using namespace cocos2d;


orgb::Color fromCocos(const cocos2d::_ccColor3B& c)
{
	return {c.r, c.g, c.b};
}

int getColorIdForSetting(const std::string& str)
{
	#define IF_STR_RET(colorstr, id) if(str == colorstr) return id;

	IF_STR_RET("BG", 1000)
	IF_STR_RET("G1", 1001)
	IF_STR_RET("Line", 1002)
	IF_STR_RET("3DL", 1003)
	IF_STR_RET("Obj", 1004)
	IF_STR_RET("P1", 1005)
	IF_STR_RET("P2", 1006)
	IF_STR_RET("LBG", 1007)
	IF_STR_RET("G2", 1009)
	IF_STR_RET("Ligther", 1012)
	IF_STR_RET("MG", 1013)
	IF_STR_RET("MG2", 1014)

	return -1;
}

int getColorIdFromSetting(int id)
{
	std::string type = fmt::format("color-{}-type", id);
	auto colorid_type = geode::Mod::get()->getSettingValue<std::string>(type);
	if(colorid_type == "ColorID") {
		return geode::Mod::get()->getSettingValue<int64_t>(fmt::format("color-{}-id", id));
	}

	return getColorIdForSetting(colorid_type);
}
std::vector<int> getRGBColorIds()
{
	std::vector<int> ret;
	
	constexpr int CURRENT_COLORS = 2;

	for(int i = 1; i <= CURRENT_COLORS; i++)
	{
		auto id = getColorIdFromSetting(i);
		if(id != -1) ret.push_back(id);
	}

	return ret;
}

orgb::Client client;
orgb::DeviceListResult deviceList;
std::vector<int> colorIds;
std::vector<cocos2d::ccColor3B> colors;

class $modify(PlayLayer)
{
	//struct Fields
	//{
	//	orgb::Client client;
	//	orgb::DeviceListResult deviceList;
	//	orgb::Device* keyboard = nullptr;
	//	std::vector<int> colorIds;
	//	std::vector<cocos2d::ccColor3B> colors;
	//};

	bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects)
	{
		if(!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		initClient();

		return true;
	}

	void setDevicesColor(const ccColor3B& c) {


		for(auto color = fromCocos(c); const auto& device : /*m_fields->*/deviceList.devices)
		{
			auto status = client.setDeviceColor(device, color);
		}
	}

	void postUpdate(float dt)
	{
		PlayLayer::postUpdate(dt);

		auto& cacheColors = /*m_fields->*/colors;

		for(int i = 0; const auto& id : /*m_fields->*/colorIds)
		{
			auto action = m_effectManager->getColorAction(id);
			if(!action)
			{
				++i;
				continue;
			}
			auto& cachedColor = cacheColors[i];

			if(auto actionColor = action->m_color; cacheColors[i] != actionColor)
			{
				cachedColor = actionColor;
				setDevicesColor(actionColor);
			}
			i++;
		}
	}

	void setupConnectedClient()
	{
		/*m_fields->*/colorIds = getRGBColorIds();

		/*m_fields->*/colors.resize(/*m_fields->*/colorIds.size());
	}

	void initClient()
	{

		if(client.isConnected())
		{
			return setupConnectedClient();
		}

		orgb::ConnectStatus status = client.connect( "127.0.0.1" );
		if (status != orgb::ConnectStatus::Success)
		{
			geode::log::info("failed to connect: %s (error code: {})\n", enumString( status ), int( client.getLastSystemError() ) );
			return;
		}

		deviceList = client.requestDeviceList();
		if (deviceList.status != orgb::RequestStatus::Success)
		{
			geode::log::info("failed to get device list: %s (error code: {})\n", enumString( status ), int( client.getLastSystemError() ) );
			return;
		}

		for(const auto& d : deviceList.devices) {
			client.changeMode(d, *d.findMode("Custom"));
			client.setDeviceColor(d, orgb::Color::Yellow);
		}

		setupConnectedClient();
	}
};  