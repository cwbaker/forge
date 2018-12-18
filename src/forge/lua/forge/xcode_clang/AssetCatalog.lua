
local AssetCatalog = forge:TargetPrototype( 'AssetCatalog' );

function AssetCatalog.create( forge, identifier, target_prototype, partial_info_plist )
	local assets = forge:File( ('%s/Assets.car'):format(identifier), AssetCatalog );
	if partial_info_plist then 
		assets:set_filename( forge:absolute(partial_info_plist), 2 );
	end
	return assets;
end

function AssetCatalog.build( forge, assets )
	local settings = forge.settings;
    local sdkroot = settings.sdkroot;
    local xcrun = settings.xcrun;

    local args = {
    	'xcrun';
    	('--sdk %s'):format( sdkroot );
    	'actool';
		'--output-format human-readable-text';
		'--compress-pngs';
		('--compile "%s"'):format( assets:ordering_dependency(1) );
	};

	local partial_info_plist = assets:filename(2);
	if partial_info_plist and partial_info_plist ~= '' then
		table.insert( args, ('--output-partial-info-plist "%s"'):format(partial_info_plist) );
	end

	table.insert( args, ('--platform %s'):format(sdkroot) );
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( args, ('--minimum-deployment-target %s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local ios_deployment_target = settings.ios_deployment_target;
        if ios_deployment_target then 
            table.insert( args, ('--minimum-deployment-target %s'):format(ios_deployment_target) );
        end
    end

	local app_icon = assets.app_icon;
	if app_icon then
		table.insert( args, ('--app-icon "%s"'):format(app_icon) );
	end

	local launch_image = assets.launch_image;
	if launch_image then 
		table.insert( args, ('--launch-image "%s"'):format(launch_image) );
	end

	table.insert( args, assets:dependency():filename() );

    forge:system( xcrun, args );
end

return AssetCatalog;
