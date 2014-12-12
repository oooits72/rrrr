Pod::Spec.new do |s|
  s.name         = "rrrr"
  s.version      = "0.0.5"
  s.summary      = "RRRR rapid real-time routing"

  s.description  = <<-DESC
RRRR (usually pronounced R4) is a C-language implementation of the RAPTOR public transit routing algorithm.
It is the core routing component of the Bliksem journey planner and passenger information system.
DESC

  s.homepage     = "https://github.com/bliksemlabs/rrrr"

  s.license      = { :type => "BSD 2-clause", :file => "LICENSE" }

  s.authors            = { "abyrd" => "andrew@fastmail.net",
                           "skinkie" => "stefan@konink.de",
                           "skywave" => "",
                           "JordenVerwer" => "" }

  s.platform     = :ios, "2.0"

  s.source       = { :git => "https://github.com/oooits72/rrrr.git", :tag => "0.0.5" }

  s.source_files = "bitset.{h,c}",
                    "config.{h,c}",
                    "geometry.{h,c}",
                    "gtfs-realtime.pb-c.{h,c}",
                    "hashgrid.{h,c}",
                    "json.{h,c}",
                    "parse.{h,c}",
                    "polyline.{h,c}",
                    "qstring.{h,c}",
                    "radixtree.{h,c}",
                    "router.{h,c}",
                    "tdata.{h,c}",
                    "util.{h,c}"

  s.requires_arc = false

  s.dependency "protobuf-c", "~> 0.15"

end
