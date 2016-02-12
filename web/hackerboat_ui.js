
/* Config vars for debugging */
var enable_map = 1;
var enable_scribble = 0;

/* backbone.js data models of what the ui is displaying */
var boatmodels = {};
boatmodels.waypoint = Backbone.Model.extend({
});
boatmodels.waypoints = Backbone.Collection.extend({
    model: boatmodels.waypoint,
});


var waypoints = new boatmodels.waypoints();
var boatstate = new Backbone.Model();

if(enable_map){
    
var renderedWaypoints = new ol.Collection();

var waypointsLayer = new ol.layer.Vector({
    source: new ol.source.Vector({features: renderedWaypoints}),
    style: new ol.style.Style({
        fill: new ol.style.Fill({
            color: 'rgba(255, 255, 255, 0.2)'
        }),
        stroke: new ol.style.Stroke({
            color: '#ffcc33',
            width: 2
        }),
        image: new ol.style.Circle({
            radius: 7,
            fill: new ol.style.Fill({
                color: '#ffcc33'
            })
        })
    })
});
var breadcrumbsLayer = new ol.layer.Vector({
});


var map = new ol.Map({
    target: 'map',
    layers: [
        /*
        new ol.layer.Tile({
            source: new ol.source.MapQuest({layer: 'sat'})
        }),
        */
        new ol.layer.Tile({
            source: new ol.source.OSM({
                url: 'http://a.tile.thunderforest.com/outdoors/{z}/{x}/{y}.png'
            })
        }),
        new ol.layer.Tile({
            source: new ol.source.XYZ({
                url: 'http://t1.openseamap.org/seamark/{z}/{x}/{y}.png'
            })
        }),

        breadcrumbsLayer,
        waypointsLayer,
    ],
    controls: [
        new ol.control.Zoom(),
        new ol.control.Rotate(),
        new ol.control.MousePosition({
            coordinateFormat: ol.coordinate.toStringHDMS,
            projection: 'EPSG:4326',
        }),
        new ol.control.ScaleLine({units: 'nautical'}),
        new ol.control.Attribution({
            collapsible: false
        }),
    ],
    view: new ol.View({
        center: ol.proj.fromLonLat([-122.79668, 48.64812 ]),
        zoom: 12
    })
});

if (enable_scribble) {
map.addInteraction(new ol.interaction.Modify({
    features: renderedWaypoints,
    // the SHIFT key must be pressed to delete vertices, so
    // that new vertices can be drawn at the same position
    // of existing vertices
    deleteCondition: function(event) {
        return ol.events.condition.shiftKeyOnly(event) &&
            ol.events.condition.singleClick(event);
    }
}));

var draw = new ol.interaction.Draw({
    features: renderedWaypoints,
    type: 'Point'
});
map.addInteraction(draw);
} else {
    map.addInteraction(new ol.interaction.Select({
        layers: [ waypointsLayer ]
    }));
}

} /* enable_map */

/* Waypoints table */
var waypointRowView = Backbone.View.extend({
    tagName: 'tr',
    initialize: function() {
        this.$el.html('<td>??</td><td>-</td><td>-</td>');
    },
    render: function(){
        var m = this.model;
        this.$el.children("td").text(function(ix, s){
            if (ix == 0) { return m.get('id'); }
            else if (ix == 1) { return ol.coordinate.toStringHDMS([m.get('lon'), m.get('lat')]); }
            else if (ix == 2) { return m.get('status'); }
        });
        return this;
    }
});
var waypointsTableView = Backbone.View.extend({
    el: '#hb-waypoints',

    initialize: function() {
        this.listenTo(this.collection, 'update reset', function(){
            this.render();
        });
    },
    
    render: function() {
        var $list = this.$('tbody').empty();

        this.collection.each(function(model) {
            var item = new waypointRowView({model: model});
            $list.append(item.render().$el);
        }, this);

        return this;
    },
    
});

if (enable_map) {
    /* Hacky binding of waypoints to the vector layer. */
    waypoints.on("reset update", function(){
        /* EPSG:4326 is the WGS84 datum we keep most points in */
        var forwardTransform = ol.proj.getTransform("EPSG:4326", map.getView().getProjection());
        renderedWaypoints.clear();
        renderedWaypoints.extend(waypoints.map(function(waypoint){
            var pos = forwardTransform([waypoint.get("lon"), waypoint.get("lat")]);
            var pt = new ol.geom.Point(pos);
            var ftr = new ol.Feature(pt);
            ftr.setId(waypoint.get("id"));
            return ftr;
        }));
    });

    waypoints.on("all", function(evn){
        // console.info("waypoints event: " + evn);
    });
}


/* Load the mock data */
$(document).ready(function(){

    $.getJSON({ url: "mock_waypoints.json",
                success: function(wpts){
                    waypoints.reset(wpts);
                }
              });

    waypointsTable = new waypointsTableView({collection: waypoints});
});

