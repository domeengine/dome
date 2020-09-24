import "graphics" for Canvas, Color
import "dome" for Json, Process

class Game {
    static init() {
      
      System.print("Test that Load from String")
      var json = Json.parse("{\"George\":\"Harrison\",\"Paul\":\"McCartney\",\"John\":\"Lennon\",\"Ringo\":\"Starr\"}").json
      
      if(!(json is Map)) {
          System.print("JSON must be a Map")
          System.print(json)
          Process.exit(-1)
      }

      if(json["George"] != "Harrison") {
          System.print("JSON was parsed badly")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Object from File")
      json = Json.fromFile("obj.test.json").json
      if(!(json is Map)) {
          System.print("JSON must be a Map")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Array from File")
      json = Json.fromFile("array.test.json").json
      if(!(json is List)) {
          System.print("JSON must be a List")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Number from File")
      json = Json.fromFile("number.test.json").json
      if(!(json is Num)) {
          System.print("JSON must be a Num")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load String from File")
      json = Json.fromFile("string.test.json").json
      
      if(!(json is String)) {
          System.print("JSON must be a String")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Null from File")
      json = Json.fromFile("null.test.json").json
      
      if(!(json is Null)) {
          System.print("JSON must be Null")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Bool from File")
      json = Json.fromFile("bool.test.json").json
      
      if(!(json is Bool)) {
          System.print("JSON must be a Bool")
          System.print(json)
          Process.exit(-1)
      }

      System.print("Test that Load Error from File")
      json = Json.fromFile("error.test.json")

      if(!(json.json is Null)) {
          System.print("json.json must be Null")
          System.print(json.json)
          Process.exit(-1)
      }

      if(!(json.error is Map)) {
          System.print("json.error must be a Map")
          System.print(json.error)
          Process.exit(-1)
      }

      System.print("Test that Load Complex from File")
      json = Json.fromFile("complex.test.json")
      
      if(!(json.json is Map)) {
          System.print("JSON must be a Map")
          System.print(json)
          Process.exit(-1)
      }

      if(json.json["uid"] != "76-buteo-albigula") {
          System.print("uid not found")
          System.print(json.json)
          Process.exit(-1)
      }

      if(json.json["name"]["latin"] != "Buteo albigula") {
          System.print("name.latin not found")
          System.print(json.json)
          Process.exit(-1)
      }

      System.print("Test that Dumps and Raw are Equal")
      json = Json.fromFile("match.test.json")
      if(json.dumps() != json.raw.trim()) {
          
          System.print("Json strings does not match")
          System.print("Dumps")
          System.print(json.dumps())
          System.print("Bytes: " + json.dumps().bytes.join())
          
          System.print("Raw")
          System.print(json.raw)
          System.print("Bytes: " + json.raw.bytes.join())
          Process.exit(-1)
      }

      Process.exit()
    }
    static update() {}
    static draw(dt) {
      Canvas.print("DOME Installed Successfully.", 10, 10, Color.white)
    }
}
