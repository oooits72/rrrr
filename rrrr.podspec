Pod::Spec.new do |s|
  s.name         = "rrrr"
  s.version      = "0.2.0"
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

  s.source       = { :git => "https://github.com/oooits72/rrrr.git", :tag => "0.2.0" }


  s.source_files = "router.{c,h}",
                    "tdata.{c,h}",
                    "tdata_validation.{c,h}",
                    "bitset.{c,h}",
                    "router_request.{c,h}",
                    "router_result.{c,h}",
                    "util.{c,h}",
                    "tdata_io_v3_mmap.c",
                    "tdata_io_v3.h",
                    "radixtree.{c,h}",
                    "geometry.{c,h}",
                    "hashgrid.{c,h}",
                    "router_dump.{c,h}",
                    "rrrr_types.h",
                    "config.h",
                    "json.{c,h}",
                    "plan_render_text.{c,h}",
                    "plan_render_otp.{c,h}",
                    "tdata_io_v4.h",
                    "tdata_io_v4_dynamic.c",
                    "tdata_io_v4_mmap.c",
                    "api.{c,h}",
                    "polyline.{c,h}",
                               "set.{c,h}"

  s.requires_arc = false

end
